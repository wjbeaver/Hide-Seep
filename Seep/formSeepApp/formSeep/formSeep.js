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

        title: 'Spring Information Form',
        style: 'width:800px',
        templateString: template,
        //        current: 'xx',
        signature: "xx",

        constructor: function (options) {
            lang.mixin(this, options);

            var sig = md5(new Date() + Math.random());
            this.signature = sig;

            //            console.log(typeof this.seepForm);
            //            var date = new Date();
            //            var dateString = date.toLocaleDateString();
            //            this.current = dateString;
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);
            console.log(typeof this.seepForm);
        },

        onChange: function (data) {
            console.log("onChange");
        },

        onSuccess: function (result) {
            console.log("onSuccess");
        },

        onError: function (evt) {
            console.log("onError");
        },

        onSubmit: function () {
            var widget = this;
            console.log("onSubmit");
            if (validate()) {
                this.submit();
            } else {
                response.status = "Validation Error";
                response.message = "<h4>Form contains invalid data.  Please correct first</h4>";
                widget.onFailure(response);
            }
        },

        onSuccess: function (response) {

        },

        onFailure: function (response) {

        }

    });
});