define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/dom",
    "dojo/on",
    "dojo/json",
    "dojo/request",
    "dojo/dom-style",
    "dojo/dom-attr",
    'formSeepApp/formSeep/namer',
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep04.html',
    "esri/geometry/Point",
    "esri/graphic",
    "dojo/date/locale",
    "esri/SpatialReference",
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dijit/form/DateTextBox",
], function (declare, lang, dojo, dom, on, JSON, request, domStyle, domAttr, namer, Dialog, _WidgetsInTemplateMixin, template, Point, Graphic, locale, SpatialReference) {
    return declare('formSeep.templates.formSeep04', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Springs Attributes',
        style: 'width:auto',
        templateString: template,
        geometry: null,
        selectedTemplate: null,
        attributes: null,
        mode: "add",
        attributeBar: null,
        url_base: appConfig.baseUrl+"data/images/",
        option: {
        		label: "Used Map App",
        		value: "App"
        	},
        options: [
                	{
                		label: "Web Map",
                		value: "WebMap"
                	},
                	{
                		label: "GPS Device",
                		value: "GPSDevice"
                	},
                	{
                		label: "Topo Map",
                		value: "TopoMap"
                	},
                	{
                		label: "Other",
                		value: "Other"
                	}
        	],
        optionType1: {
        		label: "New",
        		value: 1
        	},
        optionType2: {
        		label: "Existing",
        		value: 2
        	},
        optionType3: {
        		label: "Unknown",
        		value: 3
        	},

        newNameNode: function() {
            this.discovererNameNode.set("nameType", "Seep");
            this.discovererNameNode.set("featureValue", appConfig.layers[0].objects[0].attributes[0].value);
            this.discovererNameNode.set("newNameListTitle", "Discoverer(s)");
         	this.discovererNameNode.fillNameList();
        },
        
        namesList: function () {
            return this.discovererNameNode.namesList();
        },
        
        setForMode: function () {
            if (this.mode == "add") {
                domStyle.set(dom.byId("addButtons"), "display", "block");
                domStyle.set(dom.byId("editButtons"), "display", "none");
                this.discovererNameNode.setAdd();
            } else {
                domStyle.set(dom.byId("addButtons"), "display", "none");
                domStyle.set(dom.byId("editButtons"), "display", "block");
                this.discovererNameNode.setEdit();
            }
        },
        
        seepSelected: function (geometry, attributes, selectedTemplate) {
            this.resetForm();
            this.clearThis();
        	this.mode = "add";
        	this.setForMode();
        	
        	this.geometry = geometry;
        	this.attributes = attributes;
        	this.selectedTemplate = selectedTemplate;
        	
        	// set values and nodes
    		attributes = appConfig.layers[0].objects[0].attributes;
    		coordinates = appConfig.layers[0].objects[0].coordinates;
    		
        	attributes[0].node = this.UPLOADIDMainNode;
         	attributes[0].node.set("value", attributes[0].value);

         	coordinates.latNode = this.latitudeMainNode;
         	coordinates.latitude = this.geometry.getLatitude();
         	coordinates.latNode.set("value", coordinates.latitude);
         	
        	coordinates.longNode = this.longitudeMainNode;
         	coordinates.longitude = this.geometry.getLongitude();
         	coordinates.longNode.set("value", coordinates.longtitude);

        	attributes[1].node = this.springDateMainNode;
         	
        	if (attributes[1].value!="") {
        		attributes[1].node.set("value", attributes[1].value);
        	}
        	
       	    attributes[2].node = this.springDeviceMainNode;
        	attributes[2].value="App";
        	attributes[2].node.addOption([this.option]);
        	attributes[2].node.set("value", attributes[2].value);
     	
        	attributes[3].node = this.springFlowMainNode;
        	if (attributes[3].value!="") {
        		attributes[3].node.set("value", attributes[3].value);
        	}
        	
        	attributes[4].node = this.springTypeMainNode;
        	switch (this.attributes.TYPE) {
        		case 0:
        			attributes[4].node.addOption([this.optionType3]);
        			break;
        		case 1:
         			attributes[4].node.addOption([this.optionType1]);
        			break;
        		case 2:
         			attributes[4].node.addOption([this.optionType2]);
        			break;
        	}
        	attributes[4].node.set("value", this.attributes.TYPE);
        	
        	attributes[5].node = this.springConditionMainNode;
        	if (attributes[5].value!="") {
        		attributes[5].node.set("value", attributes[5].value);
        	}
        	
        	attributes[6].node = this.springAccuracyMainNode;
        	if (attributes[6].value!="") {
        		attributes[6].node.set("value", attributes[6].value);
        	}
        	
        	if (this.attributes.TYPE==2) {
        		domStyle.set(dom.byId("locationAccuracy"), "display", "block");
        	} else {
        		domStyle.set(dom.byId("locationAccuracy"), "display", "none");
        	}
        	
        	attributes[7].node = this.springCommentMainNode;
         	attributes[7].node.set("value", attributes[7].value);
        	
         	attributes[8].node = this.springDescribeMainNode;
         	attributes[8].node.set("value", attributes[8].value);
         	
            this.newNameNode();
        },
        
        setID: function () {
            this.resetForm();
            this.clearThis();
        	this.mode = "add";
        	this.setForMode();
        	
        	// set values and nodes
    		attributes = appConfig.layers[0].objects[0].attributes;
    		coordinates = appConfig.layers[0].objects[0].coordinates;
    		
        	attributes[0].node = this.UPLOADIDMainNode;
         	attributes[0].node.set("value", attributes[0].value);
       	
         	coordinates.latNode = this.latitudeMainNode;
         	coordinates.latNode.set("value", coordinates.latitude);
         	
        	coordinates.longNode = this.longitudeMainNode;
         	coordinates.longNode.set("value", coordinates.longitude);
        	
        	attributes[1].node = this.springDateMainNode;
         	
        	if (attributes[1].value!="") {
        		attributes[1].node.set("value", attributes[1].value);
        	}
        	
        	attributes[2].node = this.springDeviceMainNode;
        	attributes[2].node.addOption(this.options);
        	attributes[2].node.set("value", attributes[2].value);
        	
 		    attributes[3].node = this.springFlowMainNode;
        	if (attributes[3].value!="") {
        		attributes[3].node.set("value", attributes[3].value);
        	}
        	
        	attributes[4].node = this.springTypeMainNode;
        	attributes[4].node.addOption([this.optionType1, this.optionType2, this.optionType3]);
        	
        	attributes[4].node.set("value", attributes[4].value);
        	
         	if (attributes[4].node.get('value')==2) {
        		domStyle.set(dom.byId("locationAccuracy"), "display", "block");
        	} else {
        		domStyle.set(dom.byId("locationAccuracy"), "display", "none");
        	}
        	
        	attributes[5].node = this.springConditionMainNode;
        	if (attributes[5].value!="") {
        		attributes[5].node.set("value", attributes[5].value);
        	}
        	
        	attributes[6].node = this.springAccuracyMainNode;
        	if (attributes[6].value!="") {
        		attributes[6].node.set("value", attributes[6].value);
        	}
        	
        	attributes[7].node = this.springCommentMainNode;
         	attributes[7].node.set("value", attributes[7].value);
        	
         	attributes[8].node = this.springDescribeMainNode;
         	attributes[8].node.set("value", attributes[8].value);
         	
            this.newNameNode();
        },
        
        setEdit: function (attributeBar) {
		    this.discovererNameNode.clearNode();
            this.attributeBar = attributeBar;
            this.mode = "edit";
        	this.setForMode();
            
    		attributes = appConfig.layers[0].objects[0].attributes;
    		
        	attributes[0].node = this.UPLOADIDMainNode;
         	attributes[0].node.set("value", attributes[0].value);
       	         	
         	this.discovererNameNode.editNames(attributes[0].value, attributes[0].value, "Springs", attributeBar);

        	attributes[1].node = this.springDateMainNode;
         	
        	if (attributes[1].value!=null) {
        		attributes[1].node.set("value", attributes[1].value);
        	}
        	
        	attributes[2].node = this.springDeviceMainNode;
        	if (attributes[2].value=="App") {
                attributes[2].node.addOption([this.option]);
        	} else {
                attributes[2].node.addOption(this.options);
        	}
            attributes[2].node.set("value", attributes[2].value);
        	
 		    attributes[3].node = this.springFlowMainNode;
        	if (attributes[3].value!=null) {
        		attributes[3].node.set("value", attributes[3].value);
        	}
        	
        	attributes[4].node = this.springTypeMainNode;
        	
        	switch (attributes[4].value) {
        	    case 0:
      	            attributes[4].node.addOption([this.optionType3]);
        	        break;
        	    case 1:
       	            attributes[4].node.addOption([this.optionType1]);
       	        break;
        	    case 2:
      	            attributes[4].node.addOption([this.optionType2]);
        	        break;
        	    default:
       	            attributes[4].node.addOption([this.optionType1, this.optionType2, this.optionType3]);
        	}
         	
        	attributes[4].node.set("value", attributes[4].value);
        	
         	if (attributes[4].node.get('value')==2) {
        		domStyle.set(dom.byId("locationAccuracy"), "display", "block");
        	} else {
        		domStyle.set(dom.byId("locationAccuracy"), "display", "none");
        	}
        	
        	attributes[5].node = this.springConditionMainNode;
        	if (attributes[5].value!=null) {
        		attributes[5].node.set("value", attributes[5].value);
        	}
        	
        	attributes[6].node = this.springAccuracyMainNode;
        	if (attributes[6].value!=null) {
        		attributes[6].node.set("value", attributes[6].value);
        	}
        	
        	attributes[7].node = this.springCommentMainNode;
         	attributes[7].node.set("value", attributes[7].value);
        	
         	attributes[8].node = this.springDescribeMainNode;
         	attributes[8].node.set("value", attributes[8].value);
        },
        
        resetForm: function () {
    		this.springDateMainNode.set("value", null);
		    this.removeAllOptions(this.springDeviceMainNode);
            this.springFlowMainNode.set("value", 9);
            this.removeAllOptions(this.springTypeMainNode);
            this.springConditionMainNode.set("value", 9);
            this.springAccuracyMainNode.set("value", "");
            this.springCommentMainNode.set("value", "");
            this.springDescribeMainNode.set("value", "");
		    
		    this.discovererNameNode.clearNode();
       },
        
        removeAllOptions: function (node) {
        	for (i=0;i<node.options.length;i++) {
        		node.removeOption(i);
        	}        	
        },
        
        deleteImages: function () {
            appConfig.dialog_image.deleteAllImages(appConfig.layers[0].objects[0].attributes[0].value);
        },
        
        addImages: function () {
            var images = appConfig.layers[3].objects;
            var attributes
            for (i=0;i<images.length;i++) {
                attributes = images[i].attributes;
                coordinates = images[i].coordinates;
                 
        		this.attributes = new layer.attributesImage();
        		
        		this.attributes.IMAGEID_PK = attributes[0].value;
        		this.attributes.UPLOADID_FK = attributes[1].value;
        		this.attributes.Title = attributes[2].value;
                this.attributes.Description = attributes[3].value;
                this.attributes.TimeZoneName = attributes[6].value;
                this.attributes.UTC = attributes[7].value;
                
                this.attributes.Altitude = attributes[8].value;
                if (this.attributes.Altitude=="") {
                    this.attributes.Altitude = null;
                }
                
                this.attributes.Orientation = attributes[9].value;
                if (this.attributes.Orientation=="") {
                    this.attributes.Orientation = null;
                }
               
                if (this.attributes.Altitude == null) {
                    this.attributes.TYPE = 0;
                } else {
                    this.attributes.TYPE = 1;
                }
                
                this.attributes.IMAGE = this.url_base+ this.attributes.UPLOADID_FK+"/"+this.attributes.IMAGEID_PK+".jpg";

                var imageLayer = appConfig.map.getLayer("6");

                // var type = this.getType(imageLayer.types, this.attributes.TYPE);
                // var template = type.templates[0];
                
                this.selectedTemplate = {
                    featureLayer: imageLayer,
                    type: null,
                    template: null,
                    symbolInfo: null,
                    item: null,
                    showTitle: true,
                    spanlabel: false,
                    splitter: false
                }

        		this.geometry = new Point([coordinates.longitude,coordinates.latitude]);

                var newGraphic = new Graphic(this.geometry, null, this.attributes);
        	    this.selectedTemplate.featureLayer.applyEdits([newGraphic], null, null);
            }
            
            // zoom in?
            if (images.length>0) {
            }
        },
        
        sendVideoTracks: function () {
            var videos = appConfig.layers[4].objects;
            
            var send = {
                videos: [],
                tracks: []
            };

            for (i=0;i<videos.length;i++) {
                attributes = videos[i].attributes;
                coordinates = videos[i].coordinates;
                
                this.attributes = new layer.attributesVideo();
                
                this.attributes.videoid_pk = attributes[0].value;
                this.attributes.uploadid_fk = attributes[1].value;
                this.attributes.timezone = attributes[2].value;
                this.attributes.utc = attributes[3].value;
                this.attributes.type = attributes[4].value;
                this.attributes.title = attributes[5].value;
                this.attributes.url = attributes[6].value;
                this.attributes.trackid_fk = attributes[7].value;
                this.attributes.description = attributes[8].value;
                this.attributes.latitude = coordinates.latitude;
                this.attributes.longitude = coordinates.longitude;
               
                send.videos[send.videos.length] = this.attributes;
            }

            var tracks = appConfig.layers[5].objects;

            for (i=0;i<tracks.length;i++) {
                attributes = tracks[i].attributes;
                coordinates = tracks[i].coordinates;
                
                this.attributes = new layer.attributesTrack();
                
                this.attributes.trackid_pk = attributes[0].value;
                this.attributes.uploadid_fk = attributes[1].value;
                this.attributes.type = attributes[2].value;
                this.attributes.title = attributes[3].value;
                this.attributes.track = attributes[4].value;
                this.attributes.description = attributes[5].value;
                this.attributes.latitude = coordinates.latitude;
                this.attributes.longitude = coordinates.longitude;

                send.tracks[send.tracks.length] = this.attributes;
            }

        	var json = JSON.stringify(send);

            request.post(appConfig.baseUrl+"addVideoTrackFeatures.php", {
                handleAs: "json",
                method: "POST",
                headers:{'X-Requested-With': null},
                data: json
            }).then(function(response){
                console.log("The server returned: ", response);
            });
        },
        
        addVideoTracks: function () {
            videos = appConfig.layers[4].objects;
            tracks = appConfig.layers[5].objects;
            
            if (videos.length>0 || tracks.length>0) {
                this.sendVideoTracks();
            }
        },
        
        deleteVideoTracks: function () {
        },
        
        getType: function (types, value) {
            var type = null;
            for (i=0;i<types.length;i++) {
                if (types[i].id==value) {
                    type = types[i];
                    break;
                }
            }
            return type;
        },
        
        addSeepFeatures: function () {
    		attributes = appConfig.layers[0].objects[0].attributes;
    		coordinates = appConfig.layers[0].objects[0].coordinates;
    		
    		// nasty little hack
    		if (attributes[4].value==3) {
                attributes[4].value = 0;
        	}
        	
        	// check to see if a graphic has been created
        	if (this.geometry==null) {
        		// create graphic, featureLayer, attributes
        		this.attributes = new layer.attributesSeep();
        		
        		this.geometry = new Point(coordinates.longNode.value,coordinates.latNode.value);
                
                var featureLayer = appConfig.map.getLayer("5");
                var type = this.getType(featureLayer.types, attributes[4].value);
                var template = type.templates[0];
                
                this.selectedTemplate = {
                    featureLayer: featureLayer,
                    type: type,
                    template: template,
                    symbolInfo: null,
                    item: null,
                    showTitle: true,
                    spanlabel: false,
                    splitter: false
                }
        	}
        	
        	// add attributes
        	this.attributes.UPLOADID_PK = attributes[0].value;
        	this.attributes.DateFound = locale.format(attributes[1].value, {selector:"date"});
        	this.attributes.Device = attributes[2].value;
        	this.attributes.Flow = attributes[3].value;
        	this.attributes.TYPE = attributes[4].value;
        	this.attributes.Condition = attributes[5].value;
        	this.attributes.Accuracy = attributes[6].value;
        	this.attributes.Comment = attributes[7].value;
        	this.attributes.Describe = attributes[8].value;
        	this.attributes.Example = 0;
        	
        	// add point to feature layer
            var newGraphic = new Graphic(this.geometry, null, this.attributes);
        	this.selectedTemplate.featureLayer.applyEdits([newGraphic], null, null);
        	
         	// add names
        	this.discovererNameNode.addNames();
        	
        	// add image points
        	this.addImages();
        	
        	// send video and tracks to get processed
           	this.addVideoTracks();
           	
       	    this.resetForm();
        	
        	this.clearParameters();

        	this.hide();
        },
        
        editSeepFeatures: function () {
            this.discovererNameNode.updateNames(appConfig.layers[0].objects[0].attributes[0].value);
            
            var feature = this.attributeBar.currentFeature;
            
    		var attributes = appConfig.layers[0].objects[0].attributes;

            feature.attributes.UPLOADID_PK = attributes[0].value;
        	feature.attributes.DateFound = locale.format(attributes[1].value, {selector:"date"});
        	feature.attributes.Device = attributes[2].value;
        	feature.attributes.Flow = attributes[3].value;
        	feature.attributes.TYPE = attributes[4].value;
        	feature.attributes.Condition = attributes[5].value;
        	feature.attributes.Accuracy = attributes[6].value;
        	feature.attributes.Comment = attributes[7].value;
        	feature.attributes.Describe = attributes[8].value;
        	feature.attributes.Example = 0;
            
            var newGraphic = new Graphic(feature.geometry, null, feature.attributes);
            
            var seepLayer = appConfig.map.getLayer("5");

            seepLayer.applyEdits(null,[newGraphic],null);
        	
        	this.attributeBar.resetAttributes();
        	
        	this.hide();
        },
        
        deleteSeepFeatures: function () {
            this.discovererNameNode.deleteAllNames(appConfig.layers[0].objects[0].attributes[0].value);
            
            this.deleteImages();
            
            this.deleteVideoTracks();
            
            var feature = this.attributeBar.currentFeature;
            
             var newGraphic = new Graphic(feature.geometry, null, feature.attributes);
            
            var seepLayer = appConfig.map.getLayer("5");

            seepLayer.applyEdits(null,null,[newGraphic]);
            
            seepLayer = appConfig.map.getLayer("1");
            
            seepLayer.refresh();
        	
        	this.resetForm();
        	
        	this.clearParameters();
       },
        
        cleanup: function() {
        	// ask about this as a bunch of stuff could be lost
        	if (this.mode=="add") {
        	    if (confirm("Canceling will remove everything including images and tracks. Do you really want to do this?")) {
                    this.resetForm();
            
                    this.clearParameters();
                    this.hide();
        	    }
        	} else if (confirm("Canceling will remove any edits. Do you really want to do this?")) {
        	    this.hide();
        	}	    	    	    
        },
        
        clearLayers: function () {
        	// and all objects
        	for (i=0;i<appConfig.layers.length;i++) {
        		appConfig.layers[i].objects = [];
        	}
        },
        
        clearParameters: function () {
            this.clearLayers();
            
        	this.clearThis();
        },
        
        clearThis: function () {
        	this.geometry = null;
           	this.selectedTemplate = null;
           	this.attributes = null;
           	this.mode = "";
        },
        
        leaveWithAction: function () {
            // check to see if there is a name in the name list;
            if (this.discovererNameNode.checkForName(this.UPLOADIDMainNode.value)) {
                    // check to see if there is a date
                    var attributes = appConfig.layers[0].objects[0].attributes;
                if (attributes[1].node.get("value")!=null) {
                            
                        // get attributes and clear nodes
                            attributes[1].value = attributes[1].node.get("value");
                            
                            attributes[2].value = attributes[2].node.get("value");    
                            
                            attributes[3].value = attributes[3].node.get("value");
                            
                            attributes[4].value = attributes[4].node.get("value");
                            
                            attributes[5].value = attributes[5].node.get("value");
                            
                            attributes[6].value = attributes[6].node.get("value");
                            
                            attributes[7].value = attributes[7].node.get("value");
                            
                            attributes[8].value = attributes[8].node.get("value");

                        // add points and send attributes to where they belong
                        if (this.mode=="add") {
                                this.addSeepFeatures();
                        } else if (this.mode=="edit"){
                                this.editSeepFeatures();
                        }
                    } else {
                            alert("No Date!");
                    }
            } else {
                    alert("No Discoverer!");
            }           		    
        },

        constructor: function (options) {
            lang.mixin(this, options);
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);
            
            this.springTypeMainNode.on("change", lang.hitch(this, function () {
            
             	if (arguments[0]=="2") {
			    domStyle.set(dom.byId("locationAccuracy"), "display", "block");
			    
		    } else {
			    domStyle.set(dom.byId("locationAccuracy"), "display", "none");
		    }
            }));

            this.addPhotoNode.on("click", lang.hitch(this, function () {
                        var attributes = appConfig.layers[0].objects[0].attributes;
                        
                        if (attributes[1].node.get("value")!=null) {
                            attributes[1].value = attributes[1].node.get("value");
                            appConfig.dialog_image_loader.setID();
                            appConfig.dialog_image_loader.show();
                        } else {
                            alert("Needs a valid date");
                        }
            }));

            this.addVideoNode.on("click", lang.hitch(this, function () {
                        var attributes = appConfig.layers[0].objects[0].attributes;
                        
                        if (attributes[1].node.get("value")!=null) {
                            attributes[1].value = attributes[1].node.get("value");
                            appConfig.dialog_video.videoSetID();
            		        appConfig.dialog_video.show();
                        } else {
                            alert("Needs a valid date");
                        }
            }));

            this.addTrackNode.on("click", lang.hitch(this, function () {
             		    appConfig.dialog_trackSubmit.setID();
             		    appConfig.dialog_trackSubmit.show();
           }));
            
            this.editMainNode.on("click", lang.hitch(this, function () {
               this.leaveWithAction();
           }));
            
            this.submitMainNode.on("click", lang.hitch(this, function () {
                this.leaveWithAction();
            }));
        },
        
        
    });
});