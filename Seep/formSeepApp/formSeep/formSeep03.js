define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/request",
    "dojo/on",
    "dojo/dom-style",
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep03.html',
    'dijit/form/Button',
    'dojox/widget/ColorPicker',
    'dijit/form/DropDownButton',
    'dijit/form/HorizontalSlider',
    'dijit/form/Textarea'
], function (declare, lang, dojo, request, on, domStyle, Dialog, _WidgetsInTemplateMixin, template) {
    return declare('formSeep.templates.formSeep03', [Dialog, _WidgetsInTemplateMixin], {
        title: 'Image Pan',
        style: 'width: 700px;',
        templateString: template,
        annotation: false,
        image_count: 1,
        markup_count: 1,
        source: "xxx",
        xOffset: 0,
        yOffset: 0,
        markupNodes: [],
        // setters
        setSource: function (source) {
        	this.source = source;
        },
        
        setCount: function (source) {
        	this.count = count;
        },
        
        // handle marker nodes
        createMarkupNode: function (data) {
        },
        
        destroyMarkupNode: function (node) {
        },
        
        loadMarkup: function (data) {
        },
        
        postCreate: function () {
        	//make sure any parent widget's postCreate functions get called.
        	this.inherited(arguments);
        	
        	var canvas = this.__canvas = new fabric.Canvas(this.canvasNode, {
        			isDrawingMode: true,
        			width: 480,
        			height: 360
        	});
        	
        	var rect = new fabric.Rect();
        	
        	rect.toObject = (function(toObject) {
        			return function() {
        				return fabric.util.object.extend(toObject.call(this), {
        						comment: this.commentNode.containerNode.value,
        						source: this.source,
        						image_count: this.image_count,
        						markup_count: this.markup_count,
        						xOffset: this.xOffset,
        						yOffset: this.yOffset     
        				});
        			};
        	})(rect.toObject);
        	
        	canvas.add(rect);	
        	
        	fabric.Object.prototype.transparentCorners = false;
        	
        	this.annotationModeNode.on("click", lang.hitch(this, function () {
        			if (this.annotation) {
        				this.annotation = false;
        				this.annotationModeNode.containerNode.innerHTML = "Enter annotation mode";
        				domStyle.set(this.annotationNode, "display", "none");
        				domStyle.set(this.canvasNode.parentNode, "display", "none");
        				domStyle.set(this.instructionsNode, "display", "block");
        			} else {
        				this.annotation = true;
        				this.annotationModeNode.containerNode.innerHTML = "Zoom/Pan image";
        				domStyle.set(this.annotationNode, "display", "block");
        				domStyle.set(this.canvasNode.parentNode, "display", "block");
         				domStyle.set(this.instructionsNode, "display", "none");
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
        		rect.comment = this.CommentNode.value;
        		rect.xOffset = this.xOffset;
        		rect.yOffset = this.yOffset;
        		rect.source = this.source;
        		rect.image_count = this.image_count;
        		rect.markup_count = this.markup_count;
        		
        		var json = JSON.stringify(canvas);
        		
        		// save via request
        		request('/Seep/uploadImageMarkup.php', {
					handleAs: "json",
					method: "POST",
					headers:{'X-Requested-With': null},
					data: json
			}).then(function (resp) {
				this.annotateResultNode.innerHTML = resp.response;
				this.markup_count += 1;
				
				// clear
				canvas.clear();
				
				// add a div with a button that reloads the markup and a delete button right after annotationResultNode
			},
			function (error) {
				this.annotateResultNode.innerHTML = error;
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
