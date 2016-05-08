// from slate template
//https://github.com/pebble-hacks/slate-watchface-template
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://bobdotexe.github.io/watchfaces/batman_conf.html';

  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));

  if (configData.vib) {
    Pebble.sendAppMessage({
      
      fast: configData.fast,
      med: configData.med,
      slow: configData.slow,
      vib: configData.vib,
      short: configData.short,
      norm: configData.norm,
      long: configData.long,
      flash: configData.flash      
    }, function() {
      console.log('Send successful!');
    }, function() {
      console.log('Send failed!');
    });
  }
});
