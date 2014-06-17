define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep01.html',
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
    return declare('formSeep.templates.formSeep01', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Spring Image Uploader Form',
        style: 'width:600px',
        templateString: template,
        //        current: 'xx',
        signature: "xx",
        dialog_image: null,
        setDialog_image: function(value) {
            this.dialog_image = value;
        },
        constructor: function (options) {
            lang.mixin(this, options);

            this.signature = options.sig;
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

            this.uploadNode.on("error", function (evt) {
                alert(evt);
            });
            this.uploadNode.on("change", function () {
                fileList = this.getFileList();
                console.log("Change");

                for (n = 0; n < fileList.length; n++) {
                    if (this.getFileType(this.getFileList()[n].name) != 'JPG') {
                        alert('A file in the list is not a ".jpg" file!');
                        this.reset();
                        break;
                    }
                }
            });
            this.uploadNode.on("begin", lang.hitch(this, function () {
                if (!this.seepFormUpload.validate()) {
                    console.log("onSubmit false");
                    this.uploadNode.cancel();
                    // this.uploadNode.reset();
                } else {
                    console.log("onSubmit true");
                }
            }));
            this.uploadNode.on("complete", lang.hitch(this, function (response) {

                if (response.Message=="Submit Successful") {
                    this.hide();

                     // send to dialog.image
                    this.dialog_image.meta(response);
                } else {
                    alert(response.Message);
                }
            }));
        },
    });
});