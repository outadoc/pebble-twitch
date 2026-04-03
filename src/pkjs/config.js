module.exports = [
  {
    "type": "heading",
    "defaultValue": "Twitch Settings"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Credentials"
      },
      {
        "type": "text",
        "defaultValue": "Click <a href=\"https://id.twitch.tv/oauth2/authorize?client_id=53p2uwy4r832kgbxqs66g4i3idtjso&force_verify=true&redirect_uri=https%3A%2F%2Foutadoc.github.io%2Fpebble-twitch%2Fcallback.html&response_type=token&scope=user%3Aread%3Afollows\">here</a> to authorize your Twitch account to access the app, then paste your token in field below."
      },
      {
        "type": "input",
        "messageKey": "AccessToken",
        "label": "Access Token",
        "defaultValue": ""
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
