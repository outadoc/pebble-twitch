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
        "type": "button",
        "id": "btnConnectTwitch",
        "primary": true,
        "defaultValue": "Connect with Twitch"
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
