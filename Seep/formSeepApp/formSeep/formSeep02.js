define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep02.html',
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dijit/form/ComboBox',
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
    "dojo/on",
    "dojo/json"
], function (declare, lang, dojo, Dialog, _WidgetsInTemplateMixin, template, hash, date, stamp, validate, dom, on, JSON) {
    return declare('formSeep.templates.formSeep02', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Photo Form',
        style: 'width:600px;',
        templateString: template,
        images: null,
        count: 0,
        baseAddress: "http://overtexplorations.com/Seep/data/images/",
        keyGoogle: "AIzaSyAJeXarCpe7QTjf_XIrVbaAWnaoBDASGXA",
        timeZoneRequest: "https://maps.googleapis.com/maps/api/timezone/json",
        
        meta: function (images) {
            this.count = Object.keys(images).length - 2;

            this.images = images;
            
            // create the images
            for (i=0;i<this.count;i++) {
                layers[3] = addImageObject(layers[3]);
    	    }
    	
            this.imageFormFill();
            this.show();
        },
        
        getUTCSuccess: function (data, widget) {
        	var count = widget.count+1;
            var indx = layers[3].objects.length-count;
            var attributesImage = layers[3].objects[indx].attributes;
            
	        widget.DSToffsetImageNode.value = data.dstOffset;
            widget.UTCoffsetImageNode.value = data.rawOffset;
            widget.timeZoneIDImageNode.value = data.timeZoneId;
            
            attributesImage[6].value = data.timeZoneName;
            attributesImage[6].node.set("value", attributesImage[6].value);
            widget.time_zoneImageViewNode.innerHTML = attributesImage[6].value;
	    
            var GPXTime = function (offsetHours, zoneDate, zoneTime) {
                    monthDays = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
                
                    function padZero (number) {
                            out = "";
                        
                            if (number<10) {
                                   out = "0" + number; 
                            } else {
                                    out = "" + number;
                            }
                        
                            return out;
                    };
        
                hour = parseInt(zoneTime[0])+offsetHours;
            
                day = parseInt(zoneDate[2]);
            
                month = parseInt(zoneDate[1]);
            
                monthIndex = month-1;
            
                year = parseInt(zoneDate[0]);
            
                if (year % 4>0) {
                    monthDays[1]+=1;
                }
    
                if (hour>23) {
                    hour=hour%24;
                    day = day + parseInt(hour/24);
                
                    if (day>monthDays[monthIndex]) {
                        day = 1;
                        month +=1;
                        if (month>12) {
                            month = 1;
                            year += 1;
                        }
                    }
                } else if (hour<0) {
                    hour = hour + 24;
                    day = day-1;
                
                    if (day<1) {
                    if (monthIndex>0) {
                       day= monthDays[monthIndex-1];
                    } else {
                        day=monthDays[11];
                        year -= 1;
                    };
                    };
                };
            
                return year + "-" + padZero(month) + "-" + padZero(day) + "T" + padZero(hour) + ":" + zoneTime[1] + ":" + zoneTime[2] + "Z";
            };
            
	        attributesImage[7].value = GPXTime(-1*((data.dstOffset + data.rawOffset)/3600), widget.date_takenImageNode.value.split("/"), widget.time_takenImageNode.value.split(":"));
            attributesImage[7].node.set("value", attributesImage[7].value);
            widget.time_UTCImageViewNode.innerHTML = attributesImage[7].value;
            	    
            widget.UTCStatus.innerHTML = "";
       },
        
        getUTCError: function (error, widget) {
            alert(error);

            widget.UTCStatus.innerHTML = "";
        },
        
        getUTC: function (latitude, longitude,timestamp) {
            var query = "location=" + latitude + "," + longitude +
                    "&timestamp=" + timestamp/1000 +
                    "&sensor=false&key=" + this.keyGoogle;
                    
            requestGoogle(this.timeZoneRequest, query, this.getUTCSuccess, this.getUTCError, this);
        },
        
        imagePan: function () {
        	count = this.count+1;
        	dialog_imagePan.setCount(count);
        	dialog_imagePan.setSource(this.signature);
        	
            indx = layers[3].objects.length-count;
            var attributesImage = layers[3].objects[indx].attributes;
            
        	pan = "/Seep/includes/panojs/apps/panoView.html?UPLOADID=" + attributesImage[1].value + 
        		"&IMAGEID=" + attributesImage[0].value + "&width=" + this.images[this.count].width + "&height=" + this.images[this.count].height;
        	dialog_imagePan.viewerFrameNode.src = pan;

        	dialog_imagePan.show();
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
        
        imageFormFill: function () {
            if (this.count > 0) {
                var count = this.count;
                this.count--;

                this.imageNumberNode.innerHTML= "#"+count;

                indx = layers[0].objects.length-1;
                var attributes = layers[0].objects[indx].attributes;
                var coordinates = layers[0].objects[indx].coordinates;

                indx = layers[3].objects.length-count;
                var attributesImage = layers[3].objects[indx].attributes;
                var coordinatesImage = layers[3].objects[indx].coordinates;

                // add any current photographers to photographers
                var photographers = layers[4].objects;
                var nAttributes;
                
                var options = [];
                if (photographers.length>0) {
                    for (i=0;i<photographers.length;i++) {
                        nAttributes = photographers[i].attributes;
                        options[i] = {
                                            label: this.createName(nAttributes[1].valueLabel, nAttributes[2].value, nAttributes[3].value, nAttributes[4].value),
                                            value: i
                                        };
                    }
                    this.photographerNode.addOption(options);
                    this.photographerNode.set("value", 0);
               }

                // add any names of discoverers to names
                var discoverers = layers[1].objects;
                
                options = [];
                if (discoverers.length>0) {
                    for (i=0;i<discoverers.length;i++) {
                        nAttributes = discoverers[i].attributes;
                        options[i] = {
                                            label: this.createName(nAttributes[1].valueLabel, nAttributes[2].value, nAttributes[3].value, nAttributes[4].value),
                                            value: i
                                        };
                    }
                }
                
                // add any names of videographers to names
                var videographers = layers[9].objects;
                
                if (videographers.length>0) {
                    len = options.length;
                    for (i=0;i<videographers.length;i++) {
                        nAttributes = discoverers[i].attributes;
                        options[len+i] = {
                                            label: this.createName(nAttributes[1].valueLabel, nAttributes[2].value, nAttributes[3].value, nAttributes[4].value),
                                            value: i
                                        };
                    }
                }
                
                if (options.length>0) {
                    this.discovererImageNode.addOption(options);
                    this.discovererImageNode.set("value", 0);
                }
                
                // UPLOADID
                attributesImage[1].value = attributes[0].value;
                attributesImage[1].node = this.UPLOADIDImageNode;
                attributesImage[1].node.set("value", attributesImage[1].value);

                // IMAGEID
                var name = this.images.exif[this.count].FileName;
                attributesImage[0].value = name.split(".")[0];
                attributesImage[0].node = this.IMAGEIDImageNode;
                attributesImage[0].node.set("value", attributesImage[0].value);

                // portrait, landscape or square
                if (this.images[this.count].width > this.images[this.count].height) {
                    this.imageNode.width = 200;
                    this.imageNode.height = 150;
                } else if (this.images[this.count].width < this.images[this.count].height) {
                    this.imageNode.width = 150;
                    this.imageNode.height = 200;
                } else {
                    this.imageNode.width = 200;
                    this.imageNode.height = 200;
                }

                // link
                this.imageNode.src = this.baseAddress + attributesImage[1].value + "/" + "sized_" + attributesImage[0].value + "/" + attributesImage[0].value + "_medium" + ".jpg";

                // date
                if (this.images.exif[this.count].Date) {
                    attributesImage[4].value = this.images.exif[this.count].Date;
                } else {
                    attributesImage[4].value = attributes[1].vlaue;
                }
                
                this.date_takenImageViewNode.innerHTML = attributesImage[4].value;
                attributesImage[4].node = this.date_takenImageNode;
                attributesImage[4].node.set("value", attributesImage[4].value);

                // lat/long from spring or image
                if (!this.images.exif[this.count].latitude) {
                    coordinatesImage.latitude = coordinates.latitude;
                    coordinatesImage.longitude = coordinates.longitude;
                } else {
                    coordinatesImage.latitude = this.images.exif[this.count].latitude;
                    coordinatesImage.longitude = this.images.exif[this.count].longitude;
                }
                
                // title and description
                attributesImage[2].node = this.titleImageNode;
                attributesImage[3].node = this.descriptionImageNode;
               
                // altitude and heading
                if (this.images.exif[this.count].gpsAltitude) {
                    attributesImage[8].value = this.images.exif[this.count].gpsAltitude;
                    this.altitudeImageViewNode.innerHTML = attributesImage[8].value.toFixed(2)+"m.";
                    attributesImage[8].node = this.altitudeImageNode;
                    attributesImage[8].node.set("value", attributesImage[8].value);
                } else {
                    attributesImage[8].node = this.altitudeImageNode;
                    attributesImage[8].node.set("value", "");
                    this.altitudeImageViewNode.innerHTML = "Unknown";
                }

                if (this.images.exif[this.count].gpsHeading) {
                    attributesImage[9].value = this.images.exif[this.count].gpsHeading;
                    this.bearingImageViewNode.innerHTML = attributesImage[9].value.toFixed(2)+"&deg;";
                    attributesImage[9].node = this.bearingImageNode;
                    attributesImage[9].node.set("value", attributesImage[9].value);
                } else {
                    attributesImage[9].node = this.bearingImageNode;
                    attributesImage[9].node.set("value", "");
                    this.bearingImageViewNode.innerHTML = "Unknown";
                }

                // time
                // UTC has to be converted, has to bring-up a dialog because asynchronous request
                // http://www.earthtools.org/timezone/40.71417/-74.00639
                // https://maps.googleapis.com/maps/api/timezone/json?location=xx,yy&timestamp=tt&sensor=true&key=API_KEY
                attributesImage[6].node = this.timeZoneNameImageNode;
                attributesImage[7].node = this.UTCImageNode;

                if (this.images.exif[this.count].Time) {
                    attributesImage[5].value = this.images.exif[this.count].Time;
                    this.time_takenImageViewNode.innerHTML = attributesImage[5].value;
                    attributesImage[5].node = this.time_takenImageNode;
                    attributesImage[5].node.set("value", attributesImage[5].value);
    
                    this.getUTC(coordinatesImage.latitude, coordinatesImage.longitude, Date.parse(this.images.exif[this.count].Date+" "+this.images.exif[this.count].Time));
                } else {
                    this.time_takenImageViewNode.innerHTML = "Unknown";
                }
           } else {
                // going back to the main form, anything to do?
                this.hide();
            }
        },
        
        addPhotographerList: function (honorific, honorificLabel, fName, mName, lName, email) {
            indx = layers[3].objects.length-1;
            iAttributes = layers[3].objects[indx].attributes;
        
            layers[4] = addNameObject(layers[4]);
            attributes = layers[4].objects[0].attributes;
                    
            layers[2] = addNameFeatureObject(layers[2]);
            indxF = layers[2].objects.length-1;
            nfAttributes = layers[2].objects[indxF].attributes;
        
            // generate id for name
            attributes[0].value = generateUUID();

            // cross link table name id and featureid
            nfAttributes[0].value = attributes[0].value;
            nfAttributes[1].value = iAttributes[0].value;
            nfAttributes[2].value = 0;
            nfAttributes[2].valueLabel = "Photograph";

            attributes[1].value = honorific;
            attributes[1].valueLabel = honorificLabel;
            attributes[2].value = fName;
            attributes[3].value = mName;
            attributes[4].value = lName;
            attributes[5].value = email;
    
            if (this.photographerListNode.containerNode.childNodes.length>0) {
                // remove old value
                this.photographerListNode.removeOption({ 
                    value: 0 
                });
    
                this.photographerListNode.containerNode.childNodes[0].innerHTML="";
            }
            
            // add new value
            // create name
            name = this.createName(honorificLabel, fName, mName, lName);
        
            // put in selection
            var option = {
                label: name,
                value: 0
            }
        
            this.photographerListNode.addOption([option]);
            this.photographerListNode.set("value", 0);
        
            // clear photographer
            this.clearPhotographer();
        },
        
        clearPhotographer: function () {
            this.honorificPhotographerNode.set("value", "1");
            this.firstNamePhotographerNode.set("value", "");
            this.middleNamePhotographerNode.set("value", "");
            this.lastNamePhotographerNode.set("value", "");
            this.emailPhotographerNode.set("value", "");
        },
    
        removePhotographerList: function () {
            // remove from name layer object
            var object = layers[3].objects.splice(0,1);
    
            // remove name feature layer object
            var len = layers[2].objects.length-1;
            for (i=len;i>-1;i--) {		    
                if (layers[2].objects[i].attributes[0].value==object[0].attributes[0].value) {
                    layers[2].objects.splice(i,1);
                    break;
                }
             }
             
            // remove old value
            this.photographerListNode.removeOption({ 
                value: 0 
            });
    
            this.photographerListNode.containerNode.childNodes[0].innerHTML="";
        },
            
        constructor: function (options) {
            lang.mixin(this, options);

        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);
                            
           this.seepSubmitImage.on("click", lang.hitch(this, function () {
                // get and clear all the values
        	    count = this.count+1;
                indx = layers[3].objects.length-count;
                var attributesImage = layers[3].objects[indx].attributes;

                attributesImage[2].value = attributesImage[2].node.get("value");
                attributesImage[3].value = attributesImage[3].node.get("value");
                attributesImage[4].value = attributesImage[4].node.get("value");
                attributesImage[5].value = attributesImage[5].node.get("value");
                attributesImage[6].value = attributesImage[6].node.get("value");
                attributesImage[7].value = attributesImage[7].node.get("value");
                attributesImage[8].value = attributesImage[8].node.get("value");
                attributesImage[9].value = attributesImage[9].node.get("value");
                
                attributesImage[2].node.set("value", "");
                attributesImage[3].node.set("value", "");
                attributesImage[4].node.set("value", "");
                attributesImage[5].node.set("value", "");
                attributesImage[6].node.set("value", "");
                attributesImage[7].node.set("value", "");
                attributesImage[8].node.set("value", "");
                attributesImage[9].node.set("value", "");
                
                // clear names from list
               
                // continue
                this.imageFormFill();
            }));
            
            this.addPhotographerNode.on("click", lang.hitch(this, function () {
            		    
            		    // check if both are empty
            		    if (this.lastNamePhotographerNode.get("value")=="" && this.emailPhotographerNode.get("value")=="") {
            		        var value = this.discovererImageNode.get("value");
            		        
            		        var attributes;
            		        if (value<layers[1].objects.length) {
            		            // get discoverer and attributes
                                attributes = layers[1].objects[value].attributes;
                            } else {
             		           // get videographer and attributes
                               value = value-layers[1].objects.length;
                                attributes = layers[9].objects[value].attributes;
                            }
            		        
             		    	// put in photographer List
            		    	this.addPhotographerList(attributes[1].value, this.getHonorific(attributes[1].value), attributes[2].value, attributes[3].value, attributes[4].value, attributes[5].value);
            		    } else if (this.lastNamePhotographerNode.get("value")=="") {
            		    		    alert("A last name is required!");
            		    } else if (this.emailPhotographerNode.get("value")=="") {
            		    		    alert("An email address is required!");
            		    } else {
            		    	    // put in photographer List
            		    	    this.addPhotographerList(this.honorificPhotographerNode.get("value"), this.getHonorific(this.honorificPhotographerNode.get("value")), this.firstNamePhotographerNode.get("value"), this.middleNamePhotographerNode.get("value"), this.lastNamePhotographerNode.get("value"), this.emailPhotographerNode.get("value"));
            		    }
            }));

            this.removePhotographerNode.on("click", lang.hitch(this, function () {
            		    if (this.photographerListNode.get("value")>=0) {
                            // ask fer sher
                            if (confirm("Do you want to delete the selected name?")) {
                                // remove from list
                                this.removePhotographerList(this.photographerListNode.get("value"))
                            }
            		    } else {
            		    	    alert("Nothing to remove!");
            		    }
            }));

        }
    });
});