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
    'dojo/text!./formSeep04.html',
    "esri/geometry/Point",
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dijit/form/DateTextBox"
], function (declare, lang, dojo, dom, on, domStyle, domAttr, Dialog, _WidgetsInTemplateMixin, template, Point) {
    return declare('formSeep.templates.formSeep04', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Springs Attributes',
        style: 'width:auto',
        templateString: template,
        geometry: null,
        featureLayer: null,
        attributes: null,
        mode: "",
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
                		label: "GPSDevice",
                		value: "GPS Device"
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
        		value: 0
        	},

        
        seepSelected: function (geometry, attributes, featureLayer) {
        	this.mode = "add";
        	
        	this.geometry = geometry;
        	this.attributes = attributes;
        	this.featureLayer = featureLayer;
        	
        	// set values and nodes
        	indx = layers[0].objects.length-1;
    		attributes = layers[0].objects[indx].attributes;
    		coordinates = layers[0].objects[indx].coordinates;
    		
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
        },
        
        setID: function () {
        	this.mode = "add";
        	
        	// set values and nodes
        	indx = layers[0].objects.length-1;
    		attributes = layers[0].objects[indx].attributes;
    		coordinates = layers[0].objects[indx].coordinates;
    		
        	attributes[0].node = this.UPLOADIDMainNode;
         	attributes[0].node.set("value", attributes[0].value);
       	
         	coordinates.latNode = this.latitudeMainNode;
         	coordinates.latNode.set("value", coordinates.latitude);
         	
        	coordinates.longNode = this.longitudeMainNode;
         	coordinates.longNode.set("value", coordinates.longtitude);
        	
        	attributes[1].node = this.springDateMainNode;
         	
        	if (attributes[1].value!="") {
        		attributes[1].node.set("value", attributes[1].value);
        	}
        	
        	attributes[2].node = this.springDeviceMainNode;
        	attributes[2].node.addOption(this.options);
        	attributes[2].node.set("value", "WebMap");
        	
/*    		put this in editor area

        	if (attributes[2].value!="") {
        		if (attributes[2].value=="App") {
        			attributes[2].node.addOption([this.option]);
        		} else {
        			attributes[2].node.addOption(this.options);
        		}
         		attributes[2].node.set("value", attributes[2].value);
        	} else {
        		attributes[2].node.addOption(this.options);
         		attributes[2].node.set("value", "WebMap");
        	}
        	
 */        	
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
         	
        },
        
        createName: function (honorific, fName, mName, lName) {
        	
        	if (fName != "") {
        		honorific += " " + fName;
        	}
        	
        	if (mName != "") {
        		honorific += " " + mName;
        	}
        	
        	return honorific + " " + lName;
        },
        
        getHonorific: function (honorific) {
        	if (honorific!="Miss") {
        		honorific+=".";
        	}
        	return honorific
        },
        
        addDiscovererList: function (honorific, honorificLabel, fName, mName, lName, email) {
        	indx = layers[0].objects.length-1;
    		sAttributes = layers[0].objects[indx].attributes;
    		
    		layers[1] = addNameObject(layers[1]);
        	nameIndx = layers[1].objects.length-1;
        	attributes = layers[1].objects[nameIndx].attributes;
        	        	
        	layers[2] = addNameFeatureObject(layers[2]);
    		indxF = layers[2].objects.length-1;
    		nfAttributes = layers[2].objects[indxF].attributes;
    		
    		// generate id for name
        	attributes[0].value = generateUUID();

        	// cross link table name id and featureid
    		nfAttributes[0].value = attributes[0].value;
    		nfAttributes[1].value = sAttributes[0].value;
    		nfAttributes[2].value = 0;
    		nfAttributes[2].valueLabel = "Seep";

        	attributes[1].value = honorific;
        	attributes[1].valueLabel = honorificLabel;
        	attributes[2].value = fName;
        	attributes[3].value = mName;
        	attributes[4].value = lName;
         	attributes[5].value = email;
       	
        	// create name
        	name = this.createName(honorificLabel, fName, mName, lName);
        	
        	nameIndx++;
        	
        	// put in selection
        	var option = {
        		label: name,
        		value: nameIndx
        	}
        	
        	this.discovererListNode.addOption([option]);
        	this.discovererListNode.set("value", nameIndx);
        	
        	// clear discoverer
        	this.clearDiscoverer();
        },
        
        resetForm: function () {
        	indx = layers[0].objects.length-1;
    		attributes = layers[0].objects[indx].attributes;

    		attributes[1].node.set("value", "");
		this.removeAllOptions(attributes[2].node);
		attributes[3].node.set("value", 9);
		this.removeAllOptions(attributes[4].node);
		attributes[5].node.set("value", 9);
		attributes[6].node.set("value", "Verified");
		attributes[7].node.set("value", "");
		attributes[8].node.set("value", "");
		    
		this.clearDiscoverer();
       },
        
        clearDiscoverer: function () {
        	this.honorificNode.set("value", "1");
        	this.firstNameNode.set("value", "");
        	this.middleNameNode.set("value", "");
        	this.lastNameNode.set("value", "");
        	this.emailNode.set("value", "");
        },
        
        removeDiscovererList: function (value) {
            var index = value-1;
                        
            // remove from name layer object
            var object = layers[1].objects.splice(index,1);
        
            // remove name feature layer object
            var len = layers[2].objects.length-1;
            for (i=len;i>-1;i--) {		    
                if (layers[2].objects[i].attributes[0].value==object[0].attributes[0].value) {
                    layers[2].objects.splice(i,1);
                    break;
                }
            }
        
            // remove from list and select the first name
            this.discovererListNode.removeOption({ 
                    value: value 
            });
        
            if (layers[1].objects.length>0) {
                this.discovererListNode.set("value", 1);
            } else {
                // do something here that works
                if (this.discovererListNode.containerNode.length>0) {
                    this.discovererListNode.containerNode.childNodes[0].innerHTML="";
                    this.discovererListNode.containerNode.childNodes[0].textContent="";
                }
            }
        },
        
        removeAllOptions: function (node) {
        	for (i=0;i<node.options.length;i++) {
        		node.removeOption(i);
        	}
        	
        	// clear the node
            if (this.discovererListNode.containerNode.length>0) {
			    this.discovererListNode.containerNode.childNodes[0].innerHTML="";
			}
        },
        
        addSeepFeatures: function () {
        	indx = layers[0].objects.length-1;
    		attributes = layers[0].objects[indx].attributes;
    		coordinates = layers[0].objects[indx].coordinates;
        	
        	// check to see if a graphic has been created
        	if (this.geometry==null) {
        		// create graphic, eatureLayer, attributes
        		this.attributes = new layer.attributesSeep();
        		
        		this.geometry = new Point([coordinates.longitudeNode.value,coordinates.latitudeNode.value],new SpatialReference({ wkid:3857 }));

           		this.featureLayer = map.getLayer("5");
        	}
        	
        	// add attributes
        	this.attributes.UPLOADID_PK = attributes[0].value;
        	this.attributes.DateFound = attributes[1].value;
        	this.attributes.Device = attributes[2].value;
        	this.attributes.Flow = attributes[3].value;
        	this.attributes.TYPE = attributes[4].value;
        	this.attributes.Condition = attributes[5].value;
        	this.attributes.Accuracy = attributes[6].value;
        	this.attributes.Comment = attributes[7].value;
        	this.attributes.Describe = attributes[8].value;
        	
        	// add point to feature layer
        	var newGraphic = new Point(this.geometry, null, this.attributes);
        	this.featureLayer.applyEdits([newGraphic], null, null);
        	
        	this.resetForm();
        	
        	this.clearParameters();

        	// add names
        	
        	// add image points
        	
        	// send video and tracks to get processed
           	
        	this.hide();
        },
        
        editSeepFeature: function () {
        	
        	this.resetForm();
        	
        	this.clearParameters();
           	
        	this.hide();
        },
        
        deleteSeepFeature: function () {
         	
        	this.resetForm();
        	
        	this.clearParameters();
           	
        	this.hide();
       },
        
        checkForName: function () {
        	var name = false;
        	
         	for (i=0;i<layers[2].objects.length;i++) {
         		indx = layers[0].objects.length-1;
         		sAttributes = layers[0].objects[indx].attributes;
         		attributes = layers[2].objects[i].attributes
         		if (attributes[1].value==sAttributes[0].value) {
         			name = true;
         		}
         	}
         	
        	return name;
        },

        cleanup: function() {
        	// ask about this as a bunch of stuff could be lost
        	
           		    	    	    
         	this.resetForm();
        	
         	this.clearParameters();
           	
        	this.hide();
        },
        
        clearParameters: function () {
        	// and all objects ??? destructor ????
        	for (i=0;i<layers.length;i++) {
        		layers[i].objects = [];
        	}
        	
        	this.geometry = null;
           	this.featureLayer = null;
           	this.attributes = null;
           	this.mode = "";
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

            this.addNameNode.on("click", lang.hitch(this, function () {
            		    
            		    // validate
            		    if (this.lastNameNode.get("value")=="") {
            		    		    alert("A last name is required!");
            		    } else if (this.emailNode.get("value")=="") {
            		    		    alert("An email address is required!");
            		    } else {
            		    	    // put in discovererList
            		    	    console.log(this.honorificNode);
            		    	    this.addDiscovererList(this.honorificNode.get("value"), this.getHonorific(this.honorificNode.get("value")), this.firstNameNode.get("value"), this.middleNameNode.get("value"), this.lastNameNode.get("value"), this.emailNode.get("value"));
            		    }
            }));

            this.removeNameNode.on("click", lang.hitch(this, function () {
            		    if (this.discovererListNode.get("value")>=1) {
				    // ask fer sher
				    if (confirm("Do you want to delete the selected name?")) {
					    // remove from list
					    this.removeDiscovererList(this.discovererListNode.get("value"))
				    }
            		    } else {
            		    	    alert("Nothing to remove!");
            		    }
            }));

            this.addPhotoNode.on("click", lang.hitch(this, function () {
                        var indx = layers[0].objects.length-1;
                        var attributes = layers[0].objects[indx].attributes;

                        console.log(attributes[1].node.get("value"));
                        
                        if (attributes[1].node.get("value")!=null) {
                            attributes[1].value = attributes[1].node.get("value");
                            dialog_image_loader.setID();
                            dialog_image_loader.show();
                        } else {
                            alert("Needs a valid date");
                        }
            }));

            this.addVideoNode.on("click", lang.hitch(this, function () {
            		    dialog_video.show();
            }));

            this.addTrackNode.on("click", lang.hitch(this, function () {
             		    dialog_trackSubmit.show();
           }));
            
            this.submitMainNode.on("click", lang.hitch(this, function () {
            		    // check to see if there is a name in the name list;
            		    if (this.checkForName()) {
            		    	    // check to see if there is a date
            		    	    var indx = layers[0].objects.length-1;
            		    	    var attributes = layers[0].objects[indx].attributes;
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
           		    	    	    } else if (this.mode=="change"){
           		    	    	    	    this.editSeepFeatures();
           		    	    	    }
            		    	    } else {
            		    	    	    alert("No Date!");
            		    	    }
            		    } else {
            		    	    alert("No Discoverer!");
            		    }           		    
            }));
        },
        
        
    });
});