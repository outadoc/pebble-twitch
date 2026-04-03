var Clay = require('@rebble/clay');

var clayConfig = require('./config');
var clay = new Clay(clayConfig);

var MAX_STREAMS = 20;
var TWITCH_API = 'https://api.twitch.tv/helix';
var TWITCH_CLIENT_ID = '53p2uwy4r832kgbxqs66g4i3idtjso';

function xhrGet(url, headers, callback) {
  console.log('XHR GET ' + url);
  var xhr = new XMLHttpRequest();
  xhr.open('GET', url);
  for (var key in headers) {
    xhr.setRequestHeader(key, headers[key]);
  }
  xhr.onload = function () {
    console.log('XHR response: status=' + xhr.status + ' url=' + url);
    if (xhr.status === 200) {
      var data;
      try {
        data = JSON.parse(xhr.responseText);
      } catch (e) {
        console.log('XHR JSON parse error: ' + e);
        callback(new Error('JSON parse error: ' + e), null);
        return;
      }
      callback(null, data);
    } else {
      console.log('XHR error body: ' + xhr.responseText);
      callback(new Error('HTTP ' + xhr.status), null);
    }
  };
  xhr.onerror = function () {
    console.log('XHR network error for ' + url);
    callback(new Error('Network error'), null);
  };
  xhr.send();
}

function fetchLiveStreams() {
  console.log('fetchLiveStreams: reading settings');
  var settings = clay.getSettings();
  var clientId = TWITCH_CLIENT_ID;
  var accessToken = settings.AccessToken;

  console.log('fetchLiveStreams: clientId set=' + !!clientId + ' accessToken set=' + !!accessToken);

  if (!clientId || !accessToken) {
    console.log('fetchLiveStreams: no credentials configured, sending STREAM_COUNT=0');
    Pebble.sendAppMessage({ 'STREAM_COUNT': 0 });
    return;
  }

  var headers = {
    'Authorization': 'Bearer ' + accessToken,
    'Client-Id': clientId
  };

  console.log('fetchLiveStreams: fetching authenticated user');
  xhrGet(TWITCH_API + '/users', headers, function (err, data) {
    if (err) {
      console.log('fetchLiveStreams: failed to get user: ' + err);
      Pebble.sendAppMessage({ 'STREAM_COUNT': 0 });
      return;
    }

    if (!data.data || data.data.length === 0) {
      console.log('fetchLiveStreams: no user data in response');
      Pebble.sendAppMessage({ 'STREAM_COUNT': 0 });
      return;
    }

    var userId = data.data[0].id;
    console.log('fetchLiveStreams: got user id=' + userId + ' login=' + data.data[0].login);

    console.log('fetchLiveStreams: fetching followed streams for user ' + userId);
    xhrGet(TWITCH_API + '/streams/followed?user_id=' + userId + '&first=' + MAX_STREAMS, headers, function (err, streamsData) {
      if (err) {
        console.log('fetchLiveStreams: failed to get streams: ' + err);
        Pebble.sendAppMessage({ 'STREAM_COUNT': 0 });
        return;
      }

      var streams = streamsData.data || [];
      var count = Math.min(streams.length, MAX_STREAMS);
      console.log('fetchLiveStreams: got ' + streams.length + ' live streams, sending ' + count);

      Pebble.sendAppMessage({ 'STREAM_COUNT': count }, function () {
        console.log('fetchLiveStreams: STREAM_COUNT sent, starting stream data');
        sendStream(streams, 0, count);
      }, function () {
        console.log('fetchLiveStreams: failed to send STREAM_COUNT');
      });
    });
  });
}

function sendStream(streams, index, total) {
  if (index >= total) {
    console.log('sendStream: done, sent ' + total + ' streams');
    return;
  }
  var s = streams[index];
  console.log('sendStream: sending stream ' + index + '/' + total + ' user=' + s.user_name);
  var dict = {
    'STREAM_INDEX': index,
    'STREAM_USERNAME': (s.user_name || '').substring(0, 31),
    'STREAM_CATEGORY': (s.game_name || '').substring(0, 31),
    'STREAM_VIEWER_COUNT': s.viewer_count || 0,
    'STREAM_TITLE': (s.title || '').substring(0, 127)
  };
  Pebble.sendAppMessage(dict,
    function () { sendStream(streams, index + 1, total); },
    function () { console.log('sendStream: failed to send stream ' + index); }
  );
}

Pebble.addEventListener('ready', function () {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function (e) {
  console.log('AppMessage received: ' + JSON.stringify(e.payload));
  if (e.payload['REQUEST_STREAMS']) {
    //fetchLiveStreams();
  }
});
