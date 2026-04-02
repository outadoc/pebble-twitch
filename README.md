# Twitch for Pebble

A Twitch application for Pebble watches using the C SDK.

## Building

```shell
pebble build
```

## Features

The app displays your list of followed channels who are currently live.
The username is displayed, as well as the stream category and the viewer count.
When a list item is clicked, a details window opens up, displaying the same info as well as the full stream title.

## Configuration

The Twitch API credentials are stored in the app configuration using Clay for Pebble.

## Architecture

The app uses the PebbleKit JS SDK to communicate with the Twitch API. 
When opened, the watch app sends a message to the JS code running in the Pebble app.
When the JS code receives the message, it sends a request for the followed channels to the
Twitch API using the Twurple library.
