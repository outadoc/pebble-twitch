let Clay = require('@rebble/clay');

let clayConfig = require('./config');
let clay = new Clay(clayConfig);

let MAX_STREAMS = 20;
let TWITCH_API = 'https://api.twitch.tv/helix';
let TWITCH_CLIENT_ID = '53p2uwy4r832kgbxqs66g4i3idtjso';
let MOCK_API_DATA_FOR_TESTING = false;

let ERROR_NOT_CONFIGURED = -2;
let ERROR_NETWORK = -3;

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

  // Work around bug with clay.getSettings()
  let settings = JSON.parse(localStorage.getItem('clay-settings') || '{}');

  let clientId = TWITCH_CLIENT_ID;
  let accessToken = settings.AccessToken;

  console.log('fetchLiveStreams: clientId set=' + clientId + ' accessToken set=[redacted]');

  if (!clientId || !accessToken) {
    console.log('fetchLiveStreams: no credentials configured');
    Pebble.sendAppMessage({ 'STREAM_COUNT': ERROR_NOT_CONFIGURED });
    return;
  }

  let headers = {
    'Authorization': 'Bearer ' + accessToken,
    'Client-Id': clientId
  };

  console.log('fetchLiveStreams: fetching authenticated user');

  xhrGet(TWITCH_API + '/users', headers, function (err, data) {
    if (err) {
      console.log('fetchLiveStreams: failed to get user: ' + err);
      Pebble.sendAppMessage({ 'STREAM_COUNT': ERROR_NETWORK });
      return;
    }

    if (!data.data || data.data.length === 0) {
      console.log('fetchLiveStreams: no user data in response');
      Pebble.sendAppMessage({ 'STREAM_COUNT': ERROR_NETWORK });
      return;
    }

    let userId = data.data[0].id;

    console.log('fetchLiveStreams: got user id=' + userId + ' login=' + data.data[0].login);

    console.log('fetchLiveStreams: fetching followed streams for user ' + userId);

    xhrGet(TWITCH_API + '/streams/followed?user_id=' + userId + '&first=' + MAX_STREAMS, headers, function (err, streamsData) {
      if (err) {
        console.log('fetchLiveStreams: failed to get streams: ' + err);
        Pebble.sendAppMessage({ 'STREAM_COUNT': ERROR_NETWORK });
        return;
      }

      let allStreams = streamsData.data || [];
      let streams = allStreams.slice(0, MAX_STREAMS);

      console.log('fetchLiveStreams: got ' + allStreams.length + ' live streams, sending ' + streams.length);

      sendAllStreams(streams);
    });
  });
}

function sendMockStreams() {
  let mockStreams = [
    {
      "id": "1",
      "user_id": "1",
      "user_login": "loremipsum",
      "user_name": "LoremIpsum",
      "game_id": "1",
      "game_name": "NEW SUPER LOREM IPSUM BROS.",
      "type": "live",
      "title": "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin hendrerit ipsum et commodo semper. Maecenas semper lorem quis ante porta blandit. Integer porttitor, nunc ut viverra elementum, nulla nunc faucibus elit, sed tempor nisi massa nec dolor. Praesent in risus nec neque rutrum mollis. In hac habitasse platea dictumst. Phasellus porttitor ante vitae tellus convallis, at suscipit lacus vestibulum. Proin ut ligula sapien. Donec consectetur placerat nibh cursus consectetur. Donec ac est vehicula, ullamcorper dolor et, suscipit urna. Praesent ut sapien sed purus bibendum porttitor. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Vivamus id pulvinar lacus. Pellentesque venenatis magna ut magna aliquam, quis suscipit nunc consectetur.",
      "viewer_count": 9999999,
      "started_at": "2026-04-04T12:45:03Z",
      "language": "fr",
      "tag_ids": [],
      "tags": [
        "Français"
      ],
      "is_mature": false
    },
    {
      "id": "316417207651",
      "user_id": "41719107",
      "user_login": "zerator",
      "user_name": "ZeratoR",
      "game_id": "516575",
      "game_name": "VALORANT",
      "type": "live",
      "title": "MANDATORY vs GL en BO3 pour le split 2 (match 1/7 de la ligue) puis à 18h : ASCENSION PUBG 1ere qualification",
      "viewer_count": 9746,
      "started_at": "2026-04-04T12:45:03Z",
      "language": "fr",
      "tag_ids": [],
      "tags": [
        "Français"
      ],
      "is_mature": false
    },
    {
      "id": "316417090787",
      "user_id": "127129336",
      "user_login": "hortyunderscore",
      "user_name": "HortyUnderscore",
      "game_id": "18763",
      "game_name": "Fallout 3",
      "type": "live",
      "title": "sortez les pipboy",
      "viewer_count": 2047,
      "started_at": "2026-04-04T12:30:46Z",
      "language": "fr",
      "tag_ids": [],
      "tags": [
        "Français"
      ],
      "is_mature": false
    },
    {
      "id": "316417200995",
      "user_id": "62989856",
      "user_login": "shisheyu",
      "user_name": "Shisheyu",
      "game_id": "506274",
      "game_name": "Assassin's Creed Odyssey",
      "type": "live",
      "title": "POV C'EST 2018 ET LE JEU VIDÉO C’ÉTAIT MIEUX AVANT !holy",
      "viewer_count": 2036,
      "started_at": "2026-04-04T12:44:21Z",
      "language": "fr",
      "tag_ids": [],
      "tags": [
        "Français",
        "chill"
      ],
      "is_mature": false
    },
    {
      "id": "316370357218",
      "user_id": "36086436",
      "user_login": "sundae",
      "user_name": "Sundae",
      "game_id": "1329153872",
      "game_name": "Pokémon Pokopia",
      "type": "live",
      "title": "Envie de CHILL🌸 | !planning",
      "viewer_count": 680,
      "started_at": "2026-04-04T13:10:40Z",
      "language": "fr",
      "tag_ids": [],
      "tags": [
        "Français",
        "discussion",
        "découverte",
        "challenge",
        "200iq",
        "HPI",
        "chill"
      ],
      "is_mature": false
    },
    {
      "id": "316370547042",
      "user_id": "1047201773",
      "user_login": "emilien",
      "user_name": "emilien",
      "game_id": "20282",
      "game_name": "Professor Layton and the Unwound Future",
      "type": "live",
      "title": "On joue à Professeur Layton !",
      "viewer_count": 664,
      "started_at": "2026-04-04T13:33:18Z",
      "language": "fr",
      "tag_ids": [],
      "tags": [
        "Français",
        "quiz",
        "squiz",
        "kculture",
        "questionspourunchampion",
        "questions",
        "jeu",
        "gaming",
        "emilien",
        "geoguessr"
      ],
      "is_mature": false
    },
    {
      "id": "316416882531",
      "user_id": "38284441",
      "user_login": "sakor_",
      "user_name": "Sakor_",
      "game_id": "1095175715",
      "game_name": "Pokémon Z",
      "type": "live",
      "title": "POKEMON Z - L'histoire 300 ans avant X & Y, JEU DE MALADE !Discord !Subgoal",
      "viewer_count": 446,
      "started_at": "2026-04-04T12:02:58Z",
      "language": "fr",
      "tag_ids": [],
      "tags": [
        "Français",
        "Puceau",
        "Magenoir",
        "Solary",
        "Esport",
        "Gaming",
        "multigaming",
        "goat"
      ],
      "is_mature": false
    },
    {
      "id": "316794148184",
      "user_id": "538506717",
      "user_login": "marinemammalrescue",
      "user_name": "MarineMammalRescue",
      "game_id": "272263131",
      "game_name": "Animals, Aquariums, and Zoos",
      "type": "live",
      "title": "🌊 🌈 Otter Highlights-athon ─ Otters. All of them. Everywhere. And an ocean ASMR soundtrack to relax with. 🌊 🏳‍🌈",
      "viewer_count": 223,
      "started_at": "2026-04-02T19:43:04Z",
      "language": "en",
      "tag_ids": [],
      "tags": [
        "Animals",
        "ASMRvisual",
        "Rerun",
        "24HourStream",
        "Wildlife",
        "FamilyFriendly",
        "wholesome",
        "goodvibes",
        "TogetherForGood",
        "English"
      ],
      "is_mature": false
    }
  ]

  sendAllStreams(mockStreams);
}

function sendAllStreams(streams) {
  console.log(streams);
  Pebble.sendAppMessage({ 'STREAM_COUNT': streams.length }, function () {
    console.log('fetchLiveStreams: STREAM_COUNT sent, starting stream data');
    sendStream(streams, 0);
  }, function () {
    console.log('fetchLiveStreams: failed to send STREAM_COUNT');
  });
}

function sendStream(streams, index) {
  if (index >= streams.length) {
    console.log('sendStream: done, sent ' + streams.length + ' streams');
    return;
  }

  let s = streams[index];

  console.log('sendStream: sending stream ' + index + '/' + streams.length + ' user=' + s.user_name);

  let dict = {
    'STREAM_INDEX': index,
    'STREAM_USERNAME': (s.user_name || '').substring(0, 31),
    'STREAM_CATEGORY': (s.game_name || '').substring(0, 31),
    'STREAM_VIEWER_COUNT': s.viewer_count || 0,
    'STREAM_TITLE': (s.title || '').substring(0, 255)
  };

  Pebble.sendAppMessage(dict,
    function () { sendStream(streams, index + 1, streams.length); },
    function () { console.log('sendStream: failed to send stream ' + index); }
  );
}

function refreshStreams() {
  if (MOCK_API_DATA_FOR_TESTING) {
    sendMockStreams();
  } else {
    fetchLiveStreams();
  }
}

Pebble.addEventListener('ready', function () {
  console.log('PebbleKit JS ready!');
  refreshStreams();
});

Pebble.addEventListener('webviewclosed', function () {
  console.log('webviewclosed: config changed, refreshing streams');
  refreshStreams();
});

Pebble.addEventListener('appmessage', function (e) {
  console.log('AppMessage received: ' + JSON.stringify(e.payload));

  if (e.payload['REQUEST_STREAMS']) {
    refreshStreams();
  }
});
