define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep.html',
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dijit/form/ValidationTextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dojox/form/Uploader",
    "dijit/form/DateTextBox",
    "dojox/form/uploader/FileList",
    "dojo/hash",
    "dojo/date",
    "dojo/date/stamp",
    "dojox/validate/web",
    "dojox/validate/us",
    "dojo/dom",
    "dojo/on"
], function (declare, lang, dojo, Dialog, _WidgetsInTemplateMixin, template, hash, date, stamp, validate, dom, on) {
    return declare('formSeep.templates.formSeep', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Find Spring',
        style: 'width:500px',
        templateString: template,
        //        current: 'xx',
        signature: "xx",

        constructor: function (options) {
            lang.mixin(this, options);

            this.signature = options.sig;
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

            this.seepSubmit.on("click", lang.hitch(this, function () {
                if (!this.seepForm.validate()) {
                    console.log("onSubmit false");
                } else {
                    console.log("onSubmit true");

                    // get all the values
                    var coords = {
                        signature: this.signatureNode.value,
                        device: this.deviceNode.value,
                        latitude: this.latitudeNode.value,
                        longitude: this.longitudeNode.value
                    }

                    // send to map
                    // map.addSpringFromCoords(coords);
                    this.hide();
                }
            }));
        },
    });
});