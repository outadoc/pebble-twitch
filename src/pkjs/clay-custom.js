module.exports = function (minified) {
    var clayConfig = this;
    var $ = minified.$;

    function openAuthPage() {
        var AUTH_CONFIG = {
            CLIENT_ID: '53p2uwy4r832kgbxqs66g4i3idtjso',
            REDIRECT_URI: 'https://outadoc.github.io/pebble-twitch/callback.html',
            AUTH_URL: 'https://id.twitch.tv/oauth2/authorize'
        };

        var url = AUTH_CONFIG.AUTH_URL +
            '?client_id=' + AUTH_CONFIG.CLIENT_ID +
            '&redirect_uri=' + encodeURIComponent(AUTH_CONFIG.AUTH_URL) +
            '&response_type=token' +
            '&scope=user%3Aread%3Afollows';

        window.open(url, '_blank');

        alert('Authorization page opened! After authorizing:\n\n1. Copy the access token from the result page\n2. Return here and paste it in the Access Token field\n3. Click Save Settings');
    }

    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
        var connectBtn = clayConfig.getItemById('btnConnectTwitch');
        connectBtn.on('click', openAuthPage);
    });
};
