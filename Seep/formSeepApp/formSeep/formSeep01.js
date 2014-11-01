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

        title: 'Spring Photo Uploader Form',
        style: 'width:auto',
        templateString: template,
        dialog_image: null,
        setDialog_image: function (value) {
            this.dialog_image = value;
        },
        
        setID: function () {
            
        	// set values and nodes
        	indx = layers[0].objects.length-1;
    		var attributes = layers[0].objects[indx].attributes;
    		var coordinates = layers[0].objects[indx].coordinates;
    		
         	this.UPLOADIDImageUploadNode.set("value", attributes[0].value);
          	this.latitudeImageUploadNode.set("value", coordinates.latitude);
         	this.longitudeImageUploadNode.set("value", coordinates.longitude);
        },
        	
        constructor: function (options) {
            lang.mixin(this, options);

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
                    this.imageLoadStatus.innerHTML = '<img src="/images/gear.gif" width=20 height=20><span> Image is tiled after download, please wait...</span>';
                    console.log("onSubmit true");
                }
            }));
            
            this.uploadNode.on("complete", lang.hitch(this, function (response) {

                if (response.Message == "Submit Successful") {
                    this.imageLoadStatus.innerHTML = "";
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