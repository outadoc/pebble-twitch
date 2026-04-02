var Clay = require('@rebble/clay');
var clay = new Clay(require('./config'));
var ApiClient = require('@twurple/api').ApiClient;
var StaticAuthProvider = require('@twurple/auth').StaticAuthProvider;

var MAX_STREAMS = 20;

function fetchLiveStreams() {
  var settings = clay.getSettings();
  var clientId = settings.ClientID;
  var accessToken = settings.AccessToken;

  if (!clientId || !accessToken) {
    console.log('No Twitch credentials configured');
    Pebble.sendAppMessage({ 'STREAM_COUNT': 0 });
    return;
  }

  var authProvider = new StaticAuthProvider(clientId, accessToken);
  var apiClient = new ApiClient({ authProvider: authProvider });

  apiClient.users.getAuthenticatedUser(true).then(function(user) {
    console.log('Fetching followed streams for user: ' + user.id);
    return apiClient.streams.getFollowedStreamsPaginated(user.id).getPage();
  }).then(function(streams) {
    var count = Math.min(streams.length, MAX_STREAMS);
    console.log('Got ' + streams.length + ' live streams, sending ' + count);
    Pebble.sendAppMessage({ 'STREAM_COUNT': count }, function() {
      sendStream(streams, 0, count);
    }, function(e) {
      console.log('Failed to send stream count');
    });
  }).catch(function(err) {
    console.log('Error fetching streams: ' + err);
    Pebble.sendAppMessage({ 'STREAM_COUNT': 0 });
  });
}

function sendStream(streams, index, total) {
  if (index >= total) return;
  var s = streams[index];
  var dict = {
    'STREAM_INDEX': index,
    'STREAM_USERNAME': (s.userDisplayName || s.userName || '').substring(0, 31),
    'STREAM_CATEGORY': (s.gameName || '').substring(0, 31),
    'STREAM_VIEWER_COUNT': s.viewers || 0,
    'STREAM_TITLE': (s.title || '').substring(0, 127)
  };
  Pebble.sendAppMessage(dict,
    function() { sendStream(streams, index + 1, total); },
    function(e) { console.log('Failed to send stream ' + index); }
  );
}

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!');
  if (e.payload['REQUEST_STREAMS']) {
    fetchLiveStreams();
  }
});
