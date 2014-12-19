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
        	layers[0].objects[0].attributes[0].node = this.UPLOADIDNode;
         	layers[0].objects[0].coordinates.latNode = this.latitudeNode;
        	layers[0].objects[0].coordinates.longNode = this.longitudeNode;
        	layers[0].objects[0].attributes[2].node = this.deviceNode;
        	
        	// set ID
        	layers[0].objects[0].attributes[0].node.set("value", layers[0].objects[0].attributes[0].value);

        },
        
        cleanup: function() {
        	layers[0].objects = [];
        	
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
                    layers[0].objects[0].coordinates.latitude = layers[0].objects[0].coordinates.latNode.get("value");
                    layers[0].objects[0].coordinates.longitude = layers[0].objects[0].coordinates.longNode.get("value");
                    layers[0].objects[0].attributes[2].valueLabel = layers[0].objects[0].attributes[2].node.get("label");
                    layers[0].objects[0].attributes[2].value = layers[0].objects[0].attributes[2].node.get("value");
                    
                    dialog_seepMain.setID();
                    
                    this.hide();
                    
                    dialog_seepMain.show();
                }
            }));
        },
    });
});