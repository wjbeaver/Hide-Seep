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

        title: 'Find Springs',
        style: 'width:auto',
        templateString: template,

        setID: function () {
        	appConfig.layers[0].objects[0].attributes[0].node = this.UPLOADIDNode;
         	appConfig.layers[0].objects[0].coordinates.latNode = this.latitudeNode;
        	appConfig.layers[0].objects[0].coordinates.longNode = this.longitudeNode;
        	appConfig.layers[0].objects[0].attributes[2].node = this.deviceNode;
        	
        	// set ID
        	appConfig.layers[0].objects[0].attributes[0].node.set("value", appConfig.layers[0].objects[0].attributes[0].value);

        },
        
        cleanup: function() {
        	appConfig.layers[0].objects = [];
        	
        	this.hide();
        },

       constructor: function (options) {
            lang.mixin(this, options);
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

            this.seepSubmit.on("click", lang.hitch(this, function () {
                if (!this.seepForm.validate()) {
                    console.log("onSubmit false");
                } else {
                    // collect attributes and open main form
                    appConfig.layers[0].objects[0].coordinates.latitude = appConfig.layers[0].objects[0].coordinates.latNode.get("value");
                    appConfig.layers[0].objects[0].coordinates.longitude = appConfig.layers[0].objects[0].coordinates.longNode.get("value");
                    appConfig.layers[0].objects[0].attributes[2].valueLabel = appConfig.layers[0].objects[0].attributes[2].node.get("label");
                    appConfig.layers[0].objects[0].attributes[2].value = appConfig.layers[0].objects[0].attributes[2].node.get("value");
                    
                    appConfig.dialog_seepMain.setID();
                    
                    this.hide();
                    
                    appConfig.dialog_seepMain.show();
                }
            }));
        }
    });
});