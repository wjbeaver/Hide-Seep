define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/dom",
    "dojo/on",
    "dojo/dom-style",
    "dojo/dom-attr",
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep06.html',
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dijit/form/DateTextBox"
], function (declare, lang, dojo, dom, on, domStyle, domAttr, Dialog, _WidgetsInTemplateMixin, template) {
    return declare('formSeep.templates.formSeep06', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Track Upload Form',
        style: 'width:500px;',
        templateString: template,
        
        constructor: function (options) {
            lang.mixin(this, options);

            this.signature = options.sig;
        },

        setID: function () {
        
        	// set values and nodes
    		var attributes = appConfig.layers[0].objects[0].attributes;
    		var coordinates = appConfig.layers[0].objects[0].coordinates;
    		
         	this.UPLOADIDTrackUploadNode.set("value", attributes[0].value);
        },
        	
        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

            this.uploadTrackNode.on("error", function (evt) {
                alert(evt);
            });
            
            this.uploadTrackNode.on("change", function () {
                fileList = this.getFileList();
                console.log("Change");

                for (n = 0; n < fileList.length; n++) {
                    if (this.getFileType(this.getFileList()[n].name) != 'GPX') {
                        alert('A file in the list is not a ".gpx" file!');
                        this.reset();
                        break;
                    }
                }
            });
            
            this.uploadTrackNode.on("begin", lang.hitch(this, function () {
                if (!this.trackUploadFormNode.validate()) {
                    console.log("onSubmit false");
                    this.uploadTrackNode.cancel();
                } else {
                    this.trackLoadStatusNode.innerHTML = '<img src="/images/gear.gif" width=20 height=20><span> Please wait...</span>';
                    console.log("onSubmit true");
                }
            }));
            
            this.uploadTrackNode.on("complete", lang.hitch(this, function (response) {

                this.trackLoadStatusNode.innerHTML = "";
                if (response.Message == "Submit Successful") {
                    this.hide();

                    // send to dialog.image
                    appConfig.dialog_track.meta(response);
                } else {
                    alert(response.Message);
                }
            }));
        }
    });
});