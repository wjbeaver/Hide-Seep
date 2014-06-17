define([
'require',
'dojo/ready',
'dijit/registry',
'dojo/dom',
"dojo/domReady!"
], function (require, ready, registry, dom) {

    var app = {};

    var dialog;

    ready(function () {
        require(["dojo/on"], function (on) {
            on(dom.byId('btnStart'), 'click', app.onButtonClick);
        });

        console.log('loaded!');
    });

    /**
     * Display alert dialog.
     */
    app.alert = function (message) {
        require(['dijit/Dialog'], function (Dialog) {
            var dialog = new Dialog({
                title: 'Alert',
                style: 'width:400px',
                content: message
            });


            dialog.show();

            return dialog;
        });
    };

    /**
     * Display seepForm dialog.
     */
    app.seepForm = function (options) {
        require(['formSeepApp/formSeep/formSeep'], function (Dialog) {
            if (typeof this.dialog === "undefined") {
                this.dialog = new Dialog(options);
            }

            this.dialog.show();
        });
    };

    /**
     * Button click action.
     */
    app.onButtonClick = function () {
        dom.byId('loadstatus').innerHTML = "(loading ...)";
        app.seepForm({
            onSuccess: function (response) {
                console.log(response.message);
                this.onExecute(); // Hide dialog.
                app.alert('Success!');
            },
            onFailure: function (response) {
                console.log(response.message);

                if (response.status == "Validation Error") {
                    app.alert(response.message);
                } else {
                    app.alert('Failure!');
                }
            }
        });
        dom.byId('loadstatus').innerHTML = "(loaded)";
    };

    return app;
});