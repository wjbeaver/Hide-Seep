# Mozilla User Preferences

/* The data here should be appended to the user.js in the current user profile

* It can also be appended manually:
* Type about:config into the address bar and press Enter.
* Right-click -> New -> Boolean -> Name: network.protocol-handler.external.bioview -> Value -> true
* Right-click -> New -> String -> Name: network.protocol-handler.app.bioview -> Value -> /path/to/app (Replacing /path/to/app with the path to the application you want to run)
* Ensure network.protocol-handler.expose-all is set to true. 
*/

user_pref("network.protocol-handler.app.bioview", "E:\\dima\\develop\\qt4\\bioView\\msvc2008\\release\\wv.exe");
user_pref("network.protocol-handler.external.bioview", true);
