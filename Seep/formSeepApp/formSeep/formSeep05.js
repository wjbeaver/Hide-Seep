define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/dom",
    "dojo/on",
    "dojo/dom-style",
    "dojo/dom-attr",
    "dojo/date/locale",
    'dijit/Dialog',
    'formSeepApp/formSeep/UTCcall',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep05.html',
    "dijit/form/RadioButton",
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/layout/TabContainer",
    "dijit/form/TimeTextBox",
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dijit/form/DateTextBox"
], function (declare, lang, dojo, dom, on, domStyle, domAttr, locale, Dialog, UTCcall, _WidgetsInTemplateMixin, template) {
    return declare('formSeep.templates.formSeep05', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Video Form',
        style: 'width: 700px;',
        templateString: template,
        indx:   0,
        mode: "",
        
        constructor: function (options) {
            lang.mixin(this, options);

            this.signature = options.sig;
        },

        getIndexOfVideo: function (value) {
            indx = -1;
            
            var images = appConfig.layers[4].objects;
            for (i=0;i<images.length;i++) {
                var attributes = images[i].attributes;
                if (attributes[0].value == value) {
                    indx = i;
                    break;
                }
            }
            
            return indx;
        },
        
        newNameNode: function() {
            this.videographerNameNode.set("nameType", "Video");
            this.videographerNameNode.set("featureValue", appConfig.layers[0].objects[0].attributes[0].value);
            this.videographerNameNode.set("newNameListTitle", "Videographer");
            this.videographerNameNode.set("unique", true);
         	this.videographerNameNode.fillNameList();
        },
        
        addTracks: function(node) {
            var tracks = appConfig.layers[5].objects;
            var attributes;
            var options = [];
            
            for (i=0;i<tracks.length;i++) {
                attributes = tracks[i].attributes;
                option = {
                    label: attributes[3].value,
                    value: attributes[0].value
                }
                options[options.length] = option;
            }
            
            if (options.length>0) {
        	    node.addOption(options);
        	    node.set("value", options[0].value);
        	}
        },
        
        videoSetID: function () {
        	    this.mode = "add";

                var attributes = appConfig.layers[0].objects[0].attributes;
                var coordinates = appConfig.layers[0].objects[0].coordinates;
                
                // create a video object in layers
                appConfig.layers[4] = addVideoObject(appConfig.layers[4]);
                
                this.indx = appConfig.layers[4].objects.length-1;
                var attributesVideo = appConfig.layers[4].objects[this.indx].attributes;
                var coordinatesVideo = appConfig.layers[4].objects[this.indx].coordinates;

                // Coordinates
                coordinatesVideo.latitude = coordinates.latitude;
                this.coordsLatVideoViewNode.innerHTML = coordinates.latitude;
                this.latitudeVideoNode.set("value", coordinates.latitude);
                
                coordinatesVideo.longitude = coordinates.longitude;
                this.coordsLongVideoViewNode.innerHTML = coordinates.longitude;
                this.longitudeVideoNode.set("value", coordinates.longitude);

                // VIDEOID
                attributesVideo[0].value = appConfig.generateUUID();
                attributesVideo[0].node = this.VIDEOIDVideoNode;
                attributesVideo[0].node.set("value", attributesVideo[0].value);
                
                // UPLOADID
                attributesVideo[1].value = attributes[0].value;
                attributesVideo[1].node = this.UPLOADIDVideoNode;
                attributesVideo[1].node.set("value", attributesVideo[1].value);
                
                // Date
                this.videoDateNode.set("value", attributes[1].value);
                
                // Time Zone
                attributesVideo[2].node = this.timeZoneNameVideoNode;
                
                // UTC Time
                attributesVideo[3].node = this.UTCVideoNode;
                
                // TYPE
                attributesVideo[4].node = this.videoTypeNode;
                
                // Title
                attributesVideo[5].node = this.videoTitleNode;
                
                // URL
                attributesVideo[6].node = this.videoUrlNode;
                
                // Track
                attributesVideo[7].node = this.videoTrackListNode;
                this.addTracks(attributesVideo[7].node);
                
                // Description
                attributesVideo[8].node = this.videoDescribeNode;
                
                // Load Names
                this.newNameNode();

        },
        
        clearForm: function (type) {
            var attributesVideo = appConfig.layers[4].objects[this.indx].attributes;
            this.indx = 0;
            this.mode="";
            
            // Coordinates
            this.latitudeVideoNode.set("value", "");
            this.longitudeVideoNode.set("value", "");
            
            // IDS
            this.latitudeVideoNode.set("value", "");
            this.longitudeVideoNode.set("value", "");
            
            // Radio buttons
            this.coordType1Node.set("checked", true);
            domStyle.set(dom.byId("radio2"), "display", "none");
            domStyle.set(dom.byId("radio1"), "display", "block");
            
            // Names
            if (type==0) {
                this.videographerNameNode.clearNames();
            } else {
                 this.videographerNameNode.clearNode();
            }
            
            // UTC
            this.videoUTCNode.clearNode();
            
            // date
            this.videoDateNode.set("value", null);
            
            // time
            this.videoTimeNode.set("value", null);
           
            // TYPE
            attributesVideo[4].node.set("value", 1);
            
            // Title
            attributesVideo[5].node.set("value", "");
            
            // URL
            attributesVideo[6].node.set("value", "");
            
            // Track
            var indx = this.videoTrackListNode.options.length-1;
            
            for (i=indx;i>=0;i--) {
        		this.videoTrackListNode.removeOption(this.videoTrackListNode.options[i]);
        	}      	
            
            // Description
            attributesVideo[8].node.set("value", "");
        },
        
        cleanup: function () {       
            this.clearForm(0);
            appConfig.layers[4].objects.splice(this.index,1);
            this.hide();
        },
        
        addVideo: function (latitude, longitude, UTC, url) {
            var attributesVideo = appConfig.layers[4].objects[this.indx].attributes;
            var coordinatesVideo = appConfig.layers[4].objects[this.indx].coordinates;
            
                // Coordinates
                coordinatesVideo.latitude = latitude;
                coordinatesVideo.longitude = longitude;
                
                // time zone
                attributesVideo[2].value = UTC[0];
                
                // UTC
                attributesVideo[3].value = UTC[1];
                
                // TYPE
                attributesVideo[4].value = attributesVideo[4].node.get("value");
                
                // Title
                attributesVideo[5].value = attributesVideo[5].node.get("value");
                
                // url
                attributesVideo[6].value = url;

                // Track
                attributesVideo[7].value = attributesVideo[7].node.get("value");
                
                // Description
                attributesVideo[8].value = attributesVideo[8].node.get("value");
                
                this.clearForm(1);
                
                appConfig.dialog_seepMain.discovererNameNode.updateNameList();
        },
        
        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

            this.coordType1Node.on("click", lang.hitch(this, function () {
            		    domStyle.set(dom.byId("radio2"), "display", "none");
            		    domStyle.set(dom.byId("radio1"), "display", "block");
            }));

            this.coordType2Node.on("click", lang.hitch(this, function () {
            		    domStyle.set(dom.byId("radio1"), "display", "none");
             		    domStyle.set(dom.byId("radio2"), "display", "block");
            }));

            this.getVideoTrackNode.on("click", lang.hitch(this, function () {
                var value = this.videoTrackListNode.get("value");
                if (value) {
                    var attributesVideo = appConfig.layers[4].objects[this.indx].attributes;
                    attributesVideo[7].value = value;
                    
                    alert("Track added!");
                } else {
                    alert("No Tracks, upload a track then add this video");
                }             		   
           }));
            
            this.submitVideosNode.on("click", lang.hitch(this, function () {
                var latitude;
                var longitude;
                var UTC;
                var url;
                var go = true;
                
                // which radio is selected
                if (this.coordType1Node.get("checked")) {
                    latitude = this.latitudeVideoNode.get("value");
                    longitude = this.longitudeVideoNode.get("value");
                } else {
                    latitude = this.vidoeLatitudeInputNode.get("value");
                    longitude = this.vidoeLongitudeInputNode.get("value");
                    
                    if (latitude == "" || longitude=="") {
                        alert("No Input Lat/Long!");
                        go = false;
                    }
                }
                
                // has UTC
             	var UTC = this.videoUTCNode.get("UTC");
             	if (UTC.lengtn==0) {
             	    alert("Need to calculate UTC!");
             	    go = false;
             	}
             	
             	// has url
             	var url = this.videoUrlNode.get("value");
             	if (url=="") {
             	    alert("Need URL!");
             	    go = false;
             	}
             	
             	if (go) {
             	    this.addVideo(latitude, longitude, UTC, url);
             	    this.hide();
             	}
           }));
            
            this.getVideoUTCNode.on("click", lang.hitch(this, function () {
                // need lat and long
                var latitude;
                var longitude;
                var date;
                var time;
                var timestamp;
                var go = true;
                
                // which radio is selected
                if (this.coordType1Node.get("checked")) {
                    latitude = this.latitudeVideoNode.get("value");
                    longitude = this.longitudeVideoNode.get("value");
                } else {
                    latitude = this.vidoeLatitudeInputNode.get("value");
                    longitude = this.vidoeLongitudeInputNode.get("value");
                    
                    if (latitude == "" || longitude=="") {
                        alert("No Input Lat/Long!");
                        go = false;
                    }
                }
                
                // and date and time
                date = this.videoDateNode.get("value");
                if (!date) {
                        alert("No Date!");
                        go = false;
                } else {
                    date = locale.format(date, {selector:"date"});
                }
                
                time = this.videoTimeNode.get("value");
                if (!time) {
                        alert("No Time!");
                        go = false;
                } else {
                    time = locale.format(time, {selector:"time", timePattern:'HH:mm:ss'});
                }
                
                if (go) {
                    timestamp = Date.parse(date+" "+time);
                    var taken = {
                        latitude: latitude,
                        longitude: longitude,
                        timestamp: timestamp,
                        date: date,
                        time: time
                    }
                    // UTC values in widget var UTC
                    this.videoUTCNode.getUTC(taken);
                }           		    
           }));
        }
    });
});