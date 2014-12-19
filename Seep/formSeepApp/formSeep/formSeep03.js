define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/request",
    "dojo/on",
    "dojo/dom",
    "dojo/dom-style",
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep03.html',
    'dijit/form/Button',
    'dojox/widget/ColorPicker',
    'dijit/form/DropDownButton',
    'dijit/form/HorizontalSlider',
    'dijit/form/Textarea'
], function (declare, lang, dojo, request, on, dom, domStyle, Dialog, _WidgetsInTemplateMixin, template) {
    return declare('formSeep.templates.formSeep03', [Dialog, _WidgetsInTemplateMixin], {
        title: 'Annotate Photo',
        style: 'width: 700px;',
        templateString: template,
        annotation: false,
        annotations: [],
        markup_count: 1,
        mode: "",
        indx: 0,
        
        UPLOADID: "",
         _setUPLOADID: function(/*String*/ UPLOADID){
             this._set("UPLOADID", UPLOADID);
         },
         
        IMAGEID: "",
         _setIMAGEID: function(/*String*/ IMAGEID){
             this._set("IMAGEID", IMAGEID);
         },
         
        xOffset: 0,
        yOffset: 0,
        scale: 0,
        
        formClear: function () {
            this.commentNode.set("value", "");
            this.titleNode.set("value", "");
            
            if (this.mode=="edit") {
                this.annotateResultNode.innerHTML = "";
                
                // clear list
                var len = this.annotationListNode.options.length-1;
                for (i=len;i>0;i--) {
                    this.annotationListNode.removeOption(this.annotationListNode.options[i]);
                }
            }
        },
        
        cleanup: function() {
         	this.__canvas.clear();
         	this.formClear();
         	this.annotationOff();
         	this.hide();
        },
        
        setForMode: function(mode) {
            var block = dom.byId("editing-block");
            this.mode = mode;
            
            if (mode=="add") {
                domStyle.set(block, "display", "none");
            } else {
                domStyle.set(block, "display", "block");
                this.indx = 0;
                this.annotation = false;
                this.annotationModeNode.containerNode.innerHTML = "Add annotation mode";
                domStyle.set(this.annotationNode, "display", "none");
                domStyle.set(this.canvasNode.parentNode, "display", "none");
                domStyle.set(this.instructionsNode, "display", "block");
                
                var data = {
                    UPLOADID: this.UPLOADID,
                    IMAGEID: this.IMAGEID
                }
        		var json = JSON.stringify(data);
        		
        		// save via request
        		request('/Seep/getImageMarkup.php', {
					handleAs: "json",
					method: "POST",
					headers:{'X-Requested-With': null},
					data: json
                }).then(function (resp) {
                    dialog_imagePan.annotateResultNode.innerHTML = resp.response;
                    dialog_imagePan.annotations = resp.markup;
                    var options = [];
                    for (i=0;i<resp.markup.length;i++) {
                        var indx = resp.markup[i].objects.length;
                        indx--;
                        var object = resp.markup[i].objects[indx];
                        options[options.length] = {
                            label: object.title,
                            value: object.markup_count
                        }
                    }
                    
                    dialog_imagePan.annotationListNode.addOption(options);
                    dialog_imagePan.annotationListNode.set("value", 1);
                },
                function (error) {
                    dialog_imagePan.annotateResultNode.innerHTML = error;
                });
            }
        },
        getNextNode: function () {
            var len = this.annotationListNode.options.length;
            if (len>1) {
                this.annotationListNode.set("value", this.annotationListNode.options[1].value);
            } else {
                this.annotationListNode.set("value", 0);
            }
        },
        sliceAnnotationList: function(value) {
            var indx = -1;
            for (i=0;i<this.annotations;i++) {
                var len = this.annotations[i].objects.length;
                len--;
                object = this.annotations[i].objects[len];
                if (object.markup_count==value) {
                    indx = i;
                }
            }
            
            if (indx>-1) {
                this.annotations.splice(indx,1);
            }
        },
        
        annotationOff: function () {
            this.annotation = false;
            this.annotationModeNode.containerNode.innerHTML = "Add annotation mode";
            domStyle.set(this.annotationNode, "display", "none");
            domStyle.set(this.canvasNode.parentNode, "display", "none");
            domStyle.set(this.instructionsNode, "display", "block");
            
            if (this.mode=="edit") {
                domStyle.set(dom.byId("editing-block"), "display", "block");
                this.annotateImageNode.containerNode.innerHTML = "Annotate Image";
            }
        },
        
        annotationOn: function () {
            this.annotation = true;
            this.__canvas.clear();
            
            if (this.mode=="edit") {
                domStyle.set(dom.byId("editing-block"), "display", "none");
            }
            
            var IframeNode = this.viewerFrameNode;
            this.xOffset = IframeNode.contentWindow.panoXOffset;
            this.yOffset = IframeNode.contentWindow.panoYOffset;
            this.scale = IframeNode.contentWindow.panoScale;
            
            this.annotationModeNode.containerNode.innerHTML = "Zoom/Pan image";
            domStyle.set(this.annotationNode, "display", "block");
            domStyle.set(this.canvasNode.parentNode, "display", "block");
            domStyle.set(this.instructionsNode, "display", "none");
            this.annotateResultNode.innerHTML = "";
        },
        
        getShapeObject: function () {
            var shape = null;
            var value = this.annotationListNode.get("value");
            var len = -1;
            var object = null;
            
            for (i=0;i<this.annotations.length;i++) {
                len = this.annotations[i].objects.length-1;
                if (this.annotations[i].objects[len].markup_count==value) {
                    shape = this.annotations[i];
                    break;
                }
            }
            
            return shape;
        },
        
        fillEditForm: function () {
            var shape = this.getShapeObject();
            
            var len = shape.objects.length-1;
            var object = shape.objects[len];
            
            dialog_imagePan.commentNode.set("value", object.comment);
            dialog_imagePan.titleNode.set("value", object.title);
            
            // got to set the pan
            var IframeNode = this.viewerFrameNode.contentWindow;
            
            this.scale = object.scale;
            
            if (this.scale!=IframeNode.panoScale) {
                var direction = this.scale/IframeNode.panoScale;
            
                if (direction<1) {
                    direction = -1*(1/direction);
                }
            
                direction = direction/2;
            
                IframeNode.viewer1.zoom(direction);
            }
            
            var xOffOld = IframeNode.panoXOffset;
            var yOffOld = IframeNode.panoYOffset;
            
            this.xOffset = object.xOffset;
            
            this.yOffset = object.yOffset;
            
            var motion = {
                "x": (this.xOffset-xOffOld),
                "y": (this.yOffset-yOffOld)
            };
            
            IframeNode.viewer1.positionTiles(motion, true);
            
            var json = JSON.stringify(shape);
            
            this.__canvas.loadFromJSON(json, lang.hitch(this, function() {
                // making sure to render canvas at the end
                this.__canvas.renderAll();
            }));
        },
        
        getMaxMarkup: function () {
            mc = 0
            for (i=0;i<this.annotations.length;i++) {
                len = this.annotations[i].objects.length-1;
                object = this.annotations[i].objects[len];
                if (object.markup_count>mc) {
                    mc = object.markup_count;
                }
            }
            
            return mc;
        },
        
        postCreate: function () {
        	//make sure any parent widget's postCreate functions get called.
        	this.inherited(arguments);
        	
        	var canvas = this.__canvas = new fabric.Canvas(this.canvasNode, {
        			isDrawingMode: true,
        			width: 480,
        			height: 360
        	});
        	
        	fabric.Object.prototype.transparentCorners = false;
        	
        	this.editModeNode.on("click", lang.hitch(this, function () {
                this.annotateImageNode.containerNode.innerHTML = "Update";
        	    this.annotationOn();
        	    this.fillEditForm();
        	}));
        	
        	this.deleteModeNode.on("click", lang.hitch(this, function () {
        	    var value = this.annotationListNode.get("value");
        	    if (value>0) {
                    var data = {
                        UPLOADID: this.UPLOADID,
                        IMAGEID: this.IMAGEID,
                        markupCounter: value
                    }
                    var json = JSON.stringify(data);
            
                    // save via request
                    request('/Seep/deleteImageMarkup.php', {
                        handleAs: "json",
                        method: "POST",
                        headers:{'X-Requested-With': null},
                        data: json
                    }).then(function (resp) {
                        dialog_imagePan.editingResultNode.innerHTML = resp.response;
                        
                        if (resp.response=="Done!") {
                            // remove from list node
                            dialog_imagePan.editingListNode.removeOption(resp.markupCounter);
                        
                            dialog_imagePan.getNextNode();
                        
                            // remove from annotation list
                            dialog_imagePan.sliceAnnotationList(resp.markupCounter);
                        }
                    },
                    function (error) {
                        dialog_imagePan.annotateResultNode.innerHTML = error;
                    });
        	    } else {
        	        alert("No Annotation Selected!");
        	    }
        	}));
        	
        	this.annotationListNode.on("change", lang.hitch(this, function () {
        	    var indx = this.annotationListNode.get("value");
        	    indx--;
        	    this.indx = indx;
        	}));
        	
        	this.annotationModeNode.on("click", lang.hitch(this, function () {
        			if (this.annotation) {
        			    this.annotationOff();
        			} else {
        			    this.annotationOn();
         			}
        	}));
        	
        	this.clearNode.on("click", lang.hitch(this, function () { canvas.clear() }));

        	this.lineNode.on("change", lang.hitch(this, function () {
         		canvas.freeDrawingBrush.width = parseInt(this.lineNode.value, 10) || 1;
         		this.lineValueNode.innerHTML = this.lineNode.value;
         	}));
        	
        	this.colorPickerNode.on("change", lang.hitch(this, function (color) {
        		canvas.freeDrawingBrush.color = color;
        		domStyle.set(this.colorDropletNode, "backgroundColor", color);
       	    }));
        	
        	this.annotateImageNode.on("click", lang.hitch(this, function () {
        		this.annotateResultNode.innerHTML = "";

                var rect = new fabric.Rect();
                
                if (this.mode=="edit") {
                    if (this.annotateImageNode.containerNode.innerHTML == "Update") {
                        this.markup_count = this.indx+1;
                    } else {
                        this.markup_count = this.getMaxMarkup() +1;
                    }    
                }
            
                rect.toObject = (function(toObject) {
                        return function() {
                            return fabric.util.object.extend(toObject.call(this), {
                                    comment: dialog_imagePan.commentNode.get("value"),
                                    title: dialog_imagePan.titleNode.get("value"),
                                    UPLOADID: dialog_imagePan.UPLOADID,
                                    IMAGEID: dialog_imagePan.IMAGEID,
                                    markup_count: dialog_imagePan.markup_count,
                                    xOffset: dialog_imagePan.xOffset,
                                    yOffset: dialog_imagePan.yOffset,
                                    scale: dialog_imagePan.scale  
                            });
                        };
                })(rect.toObject);
            
                canvas.add(rect);	
         		
        		var json = JSON.stringify(canvas);
        		
        		// save via request
        		request('/Seep/uploadImageMarkup.php', {
					handleAs: "json",
					method: "POST",
					headers:{'X-Requested-With': null},
					data: json
                }).then(function (resp) {
                    dialog_imagePan.annotateResultNode.innerHTML = resp.response;
                    if (resp.response!="Save Failed!") {
                        if (dialog_imagePan.mode=="edit") {
                            if (dialog_imagePan.annotateImageNode.containerNode.innerHTML == "Update") {
                                // change title in list
                                dialog_imagePan.annotationListNode.options[dialog_imagePan.indx].label = dialog_imagePan.titleNode.get("value");
                            } else {
                                // add new title to list
                                var option = {
                                    label: dialog_imagePan.titleNode.get("value"),
                                    value: dialog_imagePan.markup_count
                                };
                                
                                dialog_imagePan.annotationListNode.addOption([option]);
                                dialog_imagePan.annotationListNode.set("value", dialog_imagePan.markup_count);
                             }    
                        }
                    
                        dialog_imagePan.markup_count += 1;
                
                        // clear
                        canvas.clear();
                    }
                 },
                function (error) {
                    dialog_imagePan.annotateResultNode.innerHTML = error;
                });
		}));
		
		if (canvas.freeDrawingBrush) {
        		canvas.freeDrawingBrush.color = this.colorPickerNode.value;
        		canvas.freeDrawingBrush.width = parseInt(this.lineNode.value, 10) || 1;
        		canvas.freeDrawingBrush.shadowBlur = 0;
        	};
			
        	fabric.util.addListener(fabric.window, 'load', function() {
			    var canvas = this.__canvas || this.canvas,
				canvases = this.__canvases || this.canvases;
			
			    canvas && canvas.calcOffset && canvas.calcOffset();
			
			    if (canvases && canvases.length) {
			      for (var i = 0, len = canvases.length; i < len; i++) {
				canvases[i].calcOffset();
			      }
			    }
        	});
        }
        
    });
});
