define([
    "esri/tasks/query",
    "esri/graphic",
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/dom",
    "dojo/on",
    "dojo/json",
    "dojo/dom-style",
    "dojo/request",
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
    "dojox/validate/us"
 ], function (Query, Graphic, declare, lang, dojo, dom, on, JSON, domStyle, request, Dialog, _WidgetsInTemplateMixin, 
                template, hash, date, stamp, validate) {
    return declare('formSeep.templates.formSeep02', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Photo Form',
        style: 'width:650px;',
        templateString: template,
        images: null,
        geometries: null,
        count: 0,
        mode: "",
        baseAddress: appConfig.baseUrl+"data/images/",
        keyGoogle: "AIzaSyAJeXarCpe7QTjf_XIrVbaAWnaoBDASGXA",
        timeZoneRequest: "https://maps.googleapis.com/maps/api/timezone/json",
        ignoreSet: false,
        
        meta: function (images) {
            this.count = Object.keys(images).length - 2;

            this.images = images;
            
            // create the images
            var indx = appConfig.layers[3].objects.length;
            
            for (i=0;i<this.count;i++) {               
                appConfig.layers[3] = addImageObject(appConfig.layers[3]);
                
                var name = this.images.exif[i].FileName;
                
                appConfig.layers[3].objects[indx+i].attributes[0].value = name.split(".")[0];
    	    }
    	
            this.imageFormFill();
            this.show();
        },
        
        newNameNode: function() {
            this.photographerNameNode.set("nameType", "Image");
            this.photographerNameNode.set("featureValue", this.IMAGEIDImageNode.value);
            this.photographerNameNode.set("newNameListTitle", "Photographer");
            this.photographerNameNode.set("unique", true);
         	this.photographerNameNode.fillNameList();
        },
        
        getIndexOfImage: function (value) {
            indx = -1;
            
            var images = appConfig.layers[3].objects;
            for (i=0;i<images.length;i++) {
                var attributes = images[i].attributes;
                if (attributes[0].value == value) {
                    indx = i;
                    break;
                }
            }
            
            return indx;
        },
        
        getUTCSuccess: function (data, widget) {
        	var count = widget.count+1;
            var attributesImage = appConfig.layers[3].objects[widget.getIndexOfImage(widget.IMAGEIDImageNode.value)].attributes;
            
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
        
        getWidthHeight: function (attributesImage) {
            var send = {
                uploadid: attributesImage[1].value,
                imageid: attributesImage[0].value
            };
            
            request.post(appConfig.baseUrl+"getWidthHeight.php", {
                data: send
            }).then(function(response){
                var response = JSON.parse(response);

                var pan = appConfig.baseUrl+"includes/panojs/apps/panoView.html?UPLOADID=" + response.uploadid + 
                    "&IMAGEID=" + response.imageid + "&width=" + response.width + "&height=" + response.height;
 
                appConfig.dialog_imagePan.viewerFrameNode.src = pan;

                appConfig.dialog_imagePan.setForMode("edit");
                
                appConfig.dialog_imagePan.show();
            });
        },
        
        imagePan: function () {
        	appConfig.dialog_imagePan.set("UPLOADID", this.UPLOADIDImageNode.get("value"));
        	appConfig.dialog_imagePan.set("IMAGEID", this.IMAGEIDImageNode.get("value"));
        	
            var attributesImage = appConfig.layers[3].objects[this.getIndexOfImage(this.IMAGEIDImageNode.value)].attributes;
            
            if (this.mode=="add") {
                pan = appConfig.baseUrl+"includes/panojs/apps/panoView.html?UPLOADID=" + attributesImage[1].value + 
                    "&IMAGEID=" + attributesImage[0].value + "&width=" + this.images[this.count].width + "&height=" + this.images[this.count].height;
            
                appConfig.dialog_imagePan.viewerFrameNode.src = pan;
                
                appConfig.dialog_imagePan.setForMode("add");
                
                appConfig.dialog_imagePan.show();
        	} else {
        	    this.getWidthHeight(attributesImage);
        	}
	    },
	    
	    setForMode: function () {
            if (this.mode == "add") {
                domStyle.set(dom.byId("photoList"), "display", "none");
                domStyle.set(dom.byId("updatePhoto"), "display", "none");
                domStyle.set(dom.byId("addPhoto"), "display", "block");
                domStyle.set(dom.byId("imageTaken"), "display", "block");
                domStyle.set(dom.byId("imageViewClose"), "display", "none");
                this.photographerNameNode.setAdd();
            } else {
                domStyle.set(dom.byId("photoList"), "display", "block");
                domStyle.set(dom.byId("updatePhoto"), "display", "block");
                domStyle.set(dom.byId("addPhoto"), "display", "none");
                domStyle.set(dom.byId("imageTaken"), "display", "none");
                domStyle.set(dom.byId("imageViewClose"), "display", "block");
                this.photographerNameNode.setEdit();
            }
	    },
	    
	    clearEditForm: function () {
            var indx = this.photoListNode.options.length-1;
            
            for (i=indx;i>=0;i--) {
        		this.photoListNode.removeOption(this.photoListNode.options[i]);
        	}      	

            this.photoListNode.addOption({
                label: "Photos",
                value: 0
            });
            
            this.ignoreSet = true;
            this.photoListNode.set("value", 0);
            
            this.imageNode.src = "";
            
            this.titleImageNode.set("value", "");
            this.descriptionImageNode.set("value", "");
            
            this.date_takenImageViewNode.innerHTML = "";
            this.time_takenImageViewNode.innerHTML = "";
            this.time_zoneImageViewNode.innerHTML = "";
            this.time_UTCImageViewNode.innerHTML = "";
            this.altitudeImageViewNode.innerHTML = "";
            this.bearingImageViewNode.innerHTML = "";
            
            this.photographerNameNode.clearNode();
            
            this.UPLOADIDImageNode.set("value", "");
            this.IMAGEIDImageNode.set("value", "");
            this.latitudeImageNode.set("value", "");
            this.longitudeImageNode.set("value", "");
            this.date_takenImageNode.set("value", null);
            this.time_takenImageNode.set("value", null);
            this.timeZoneNameImageNode.set("value", "");
            this.timeZoneIDImageNode.set("value", "");
            this.DSToffsetImageNode.set("value", "");
            this.UTCoffsetImageNode.set("value", "");
            this.UTCImageNode.set("value", "");
            this.altitudeImageNode.set("value", null);
            this.bearingImageNode.set("value", null);
	    },
	    
	    fillEditForm: function (indx, start) {
	    
 	       var image = this.images[indx];
 	       
 	       this.count = indx;
 	       
 	       var attributes = appConfig.layers[3].objects[indx].attributes
	        
           this.imageNumberNode.innerHTML= "#"+indx+1;
            
            if (start) {
                var options = [];
                var value;
                for (i=0;i<this.images.length;i++) {
                    options[options.length] = {
                        label: this.images[i].Title,
                        value: this.images[i].IMAGEID_PK
                    };
                    if (i==indx) {
                        value = this.images[i].IMAGEID_PK;
                    }
	        }
	        
	        this.photoListNode.addOption(options);
	        
	        this.ignoreSet = true;
            this.photoListNode.set("value", value);
            }
            // link
            this.imageNode.src = this.baseAddress + image.UPLOADID_FK + "/" + "sized_" + image.IMAGEID_PK + "/" + image.IMAGEID_PK + "_small" + ".jpg";
            attributes[0].value = image.IMAGEID_PK;
            attributes[0].node = this.IMAGEIDImageNode
            attributes[0].node.set("value", attributes[0].value);
            
            attributes[1].value = image.UPLOADID_FK;
            attributes[1].node = this.UPLOADIDImageNode
            attributes[1].node.set("value", attributes[1].value);
            
            attributes[2].node = this.titleImageNode;
            attributes[2].node.set("value", image.Title);
            attributes[2].value = image.Title;
            
            attributes[3].node = this.descriptionImageNode;
            attributes[3].node.set("value", image.Description);
            attributes[3].value = image.Description;
            
            this.time_zoneImageViewNode.innerHTML = image.TimeZoneName;
            this.time_UTCImageViewNode.innerHTML = image.UTC;
            
            if (image.Altitude==null) {
                this.altitudeImageViewNode.innerHTML = "";
            } else {
                this.altitudeImageViewNode.innerHTML = image.Altitude;
            }
            
            if (image.Orientation==null) { 
                this.bearingImageViewNode.innerHTML = "";
            } else {
                this.bearingImageViewNode.innerHTML = image.Orientation;
            }
            
            this.UPLOADIDImageNode.set("value", image.UPLOADID_FK);
            this.IMAGEIDImageNode.set("value", image.IMAGEID_PK);

            this.photographerNameNode.set("nameType", "Image");
            this.photographerNameNode.set("featureValue", this.IMAGEIDImageNode.value);
            this.photographerNameNode.set("newNameListTitle", "Photographer");
            this.photographerNameNode.set("unique", true);
            this.photographerNameNode.editNames(image.UPLOADID_FK, image.IMAGEID_PK, "Image");
	    },
	    
	    fetchRecords: function(featureSet) {
	        this.images = [];
	        this.geometries = [];
	        if (featureSet.features.length > 0) {
	            for (i=0;i<featureSet.features.length;i++) {
	                this.images[i] = featureSet.features[i].attributes;
	                this.geometries[i] = featureSet.features[i].geometry;
	                appConfig.layers[3] = addImageObject(appConfig.layers[3]);
	            }
	            
	            // clear the form
	            this.clearEditForm();
	            
	            // fill the form with the first image
	            this.fillEditForm(0, true);
	            
	            this.show();
	        } else {
	            alert("No Images!");
	        }
	    },
	    
	    imageSetEdit: function (UPLOADID) {
            this.mode = "edit";
            this.setForMode();
            
            // query for images
            var layer = appConfig.map.getLayer("6");
            var query = new Query();
            query.returnGeometry = true;
            query.outFields = ["*"];
            query.where = "UPLOADID_FK='"+UPLOADID+"'";
            
            layer.queryFeatures(query, lang.hitch(this, function (featureSet) {
                this.fetchRecords(featureSet);
            }));
	    },
	    
	    fetchAndDeleteAll: function(featureSet) {
	        if (featureSet.features.length > 0) {
	            var deletes = [];
	            for (i=0;i<featureSet.features.length;i++) {
	                var image = featureSet.features[i].attributes;
	                var geometry = featureSet.features[i].geometry;
            
                    var layer = appConfig.map.getLayer("6");
                    
                    this.photographerNameNode.deleteAllNames(image.IMAGEID_PK);
            
                    deletes[deletes.length] = new Graphic(geometry, null, image);        
	            }
	            
	            layer.applyEdits(null,null,deletes);
            
                imageLayer = appConfig.map.getLayer("2");
            
                imageLayer.refresh();
	        }
	    },
	    
	    deleteAllImages: function (UPLOADID) {
            // query for images
            var layer = appConfig.map.getLayer("2");
            var query = new Query();
            query.returnGeometry = true;
            query.outFields = ["*"];
            query.where = "UPLOADID_FK='"+UPLOADID+"'";
            
            layer.queryFeatures(query, lang.hitch(this, function (featureSet) {
                this.fetchAndDeleteAll(featureSet);
            }));
	    },
        
        imageFormFill: function () {
            if (this.count > 0) {
        	    this.mode = "add";
        	    this.setForMode();
        	
                var count = this.count;
                this.count--;

                this.imageNumberNode.innerHTML= "#"+count;

                var attributes = appConfig.layers[0].objects[0].attributes;
                var coordinates = appConfig.layers[0].objects[0].coordinates;

                var name = this.images.exif[this.count].FileName;
                
                var indx = this.getIndexOfImage(name.split(".")[0]);
                
                var attributesImage = appConfig.layers[3].objects[indx].attributes;
                var coordinatesImage = appConfig.layers[3].objects[indx].coordinates;

                // UPLOADID
                attributesImage[1].value = attributes[0].value;
                attributesImage[1].node = this.UPLOADIDImageNode;
                attributesImage[1].node.set("value", attributesImage[1].value);

                // IMAGEID
                attributesImage[0].node = this.IMAGEIDImageNode;
                attributesImage[0].node.set("value", attributesImage[0].value);

                // link
                this.imageNode.src = this.baseAddress + attributesImage[1].value + "/" + "sized_" + attributesImage[0].value + "/" + attributesImage[0].value + "_small" + ".jpg";

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
                    this.altitudeImageViewNode.innerHTML = attributesImage[8].value.toFixed(2)+" m.";
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
                    attributesImage[5].node = this.time_takenImageNode;
                   this.time_takenImageViewNode.innerHTML = "Unknown";
                }
                
                this.newNameNode();
           } else {
                // going back to the main form, name fix
                this.photographerNameNode.clearNode();
                appConfig.dialog_seepMain.discovererNameNode.updateNameList();
                
                this.hide();
            }
        },
        
        getImageIndex: function (value) {
            var indx = -1;
            for (i=0;i<this.images.length;i++) {
                if (this.images[i].IMAGEID_PK==value) {
                    indx=i;
                    break;
                }
            }
            
            return indx;
        },
        
        changePhoto: function () {
            if (!this.ignoreSet) {
                var value = this.photoListNode.get("value");
            
                if (value == 0) {
                    alert("Select a photo!");
                } else {
                    var indx = this.getImageIndex(value);
                    this.fillEditForm(indx, false);
                }
            } else {
                this.ignoreSet = false;
            }
        },
        
        updateImage: function () {
            // list of photos
            var indx = this.count+1;
            this.photoListNode.options[indx].label = this.titleImageNode.get("value");
            this.ignoreSet = true;
            this.photoListNode.set("value", this.photoListNode.get("value"));
            
            // just update photographer
            this.photographerNameNode.updateNames(null);
            
            // update title and description
            var attributesImage = appConfig.layers[3].objects[this.getIndexOfImage(this.IMAGEIDImageNode.value)].attributes;

            attributesImage[2].value = attributesImage[2].node.get("value");
            attributesImage[3].value = attributesImage[3].node.get("value");
            
            indx--;
 	        var image = this.images[indx];
 	        var geometry = this.geometries[indx];
            
            image.Title = attributesImage[2].value;
            image.Description = attributesImage[3].value;
            
            var layer = appConfig.map.getLayer("6");
            
            var newGraphic = new Graphic(geometry, null, image);
            
            layer.applyEdits(null,[newGraphic],null);
        },
        
        deleteImage: function () {
            // delete from database
            var indx = this.count;
            
 	        var image = this.images[indx];
 	        var geometry = this.geometries[indx];
            
            var layer = appConfig.map.getLayer("6");
            
            var newGraphic = new Graphic(geometry, null, image);
            
            layer.applyEdits(null,null,[newGraphic]);
            
            imageLayer = appConfig.map.getLayer("2");
            
            imageLayer.refresh();

            // delete feature names
            this.photographerNameNode.deleteNames(image.IMAGEID_PK);

            // delete from geometries/ images
            this.geometries.splice(indx,1);
            this.images.splice(indx,1);
           
            // delete from layers
            appConfig.layers[3].objects.splice(this.getIndexOfImage(image.IMAGEID_PK),1);
            
            // delete from list
            this.photoListNode.removeOption(this.photoListNode.get("value"));
            
            if (this.photoListNode.options.length>1) {
                for (i=0;i<this.photoListNode.options.length;i++) {
                    if (this.photoListNode.options[i].value!=0) {
                        this.photoListNode.set("value", this.photoListNode.options[i].value);
                        break;
                    }
                }
            } else {
                this.clearEditForm();
                this.hide();
            }
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
                var attributesImage = appConfig.layers[3].objects[this.getIndexOfImage(this.IMAGEIDImageNode.value)].attributes;

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
                
                // clear names from list and update name list
               this.photographerNameNode.clearNode();
                              
                // continue
                this.imageFormFill();
            }));
            
            this.imageViewCloseNode.on("click", lang.hitch(this, function () {
                if (confirm("Closing this will remove all changes that haven't been updated. OK?")) {
                    this.clearEditForm();
                    this.hide();
                }
            }));

           this.seepEditImage.on("click", lang.hitch(this, function () {
                if (confirm("This will update the image. OK?")) {
                    this.updateImage();
                }
            }));
            
           this.seepDeleteImage.on("click", lang.hitch(this, function () {
                if (confirm("This will delete the image. OK?")) {
                    this.deleteImage();
                }
            }));
            
           this.photoListNode.on("change", lang.hitch(this, function () {
                this.changePhoto();
            }));
        }
    });
});