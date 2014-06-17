var dialog_image_loader;
var dialog_image;
var dialog;

define([
'require',
'dojo/ready',
'dijit/registry',
'dojo/dom',
"dojo/domReady!"
], function (require, ready, registry, dom) {

    var app = {};

    ready(function () {
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
            },
            sig: md5(new Date() + Math.random())
        });

        require(["dojo/on"], function (on) {
            on(dom.byId('btnStart'), 'click', app.onButtonClick);
        });

        dom.byId('loadstatus').innerHTML = "(loaded)";
        console.log('loaded!');
    });

    /**
     * Create seepForm dialog.
     */
    app.seepForm = function (options) {
        require(['formSeepApp/formSeep/formSeep', 'formSeepApp/formSeep/formSeep01', 'formSeepApp/formSeep/formSeep02'], function (Dialog, Dialog_image_loader, Dialog_image) {
            if (typeof dialog === "undefined") {
                dialog = new Dialog(options);
            }

            if (typeof dialog_image_loader === "undefined") {
                dialog_image_loader = new Dialog_image_loader(options);
            }

            if (typeof dialog_image === "undefined") {
                options.closable = false;
                dialog_image = new Dialog_image(options);
            }
        });
    };

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

            return dialog;
        });
    };
    /**
     * Button click action.
     */
    app.onButtonClick = function () {
        dialog.show();
    };
    return app;
});