require([
	"dojo/request",
	"dojo/domReady!"
	], function (request) {
		(function() {
			var $ = function(id){return document.getElementById(id)};

			var canvas = this.__canvas = new fabric.Canvas('c', {
					isDrawingMode: true
			});

			var rect = new fabric.Rect();

			rect.toObject = (function(toObject) {
			  return function() {
			    return fabric.util.object.extend(toObject.call(this), {
			      comment: this.comment,
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
			
			var drawingModeEl = $('drawing-mode'),
			      drawingOptionsEl = $('drawing-mode-options'),
			      drawingColorEl = $('drawing-color'),
			      drawingLineWidthEl = $('drawing-line-width'),
			      drawingCommentEl = $('drawing-comment'),
			      clearEl = $('clear-canvas'),
			      cEl = $('c'),
			      annotateEl = $('annotate'),
			      annotateResultEl = $('annotate-result');
			
			clearEl.onclick = function() { canvas.clear() };
			  
			annotateEl.onclick = function() { 
				annotateResultEl.innerHTML = "";
				rect.comment = drawingCommentEl.value;
				rect.xOffset = 0;
				rect.yOffset = 0;
				rect.source = "xxx";
				rect.image_count = 1;
				rect.markup_count = 1;
				
				var json = JSON.stringify(canvas);
				
				// save via request
				request('/Seep/uploadImageMarkup.php', {
					handleAs: "json",
					method: "POST",
					headers:{'X-Requested-With': null},
					data: json
				}).then(function (resp) {
						annotateResultEl.innerHTML = resp.response;
					},
					function (error) {
						annotateResultEl.innerHTML = error;
					});
			};

			drawingModeEl.onclick = function() {
			    canvas.isDrawingMode = !canvas.isDrawingMode;
			    if (canvas.isDrawingMode) {
			      drawingModeEl.innerHTML = 'Cancel annotation mode';
			      drawingOptionsEl.style.display = '';
			      cEl.style.display = '';
			   }
			    else {
			      drawingModeEl.innerHTML = 'Enter annotation mode';
			      drawingOptionsEl.style.display = 'none';
			      cEl.style.display = 'none';
			    }
			};
			
			
			$('drawing-mode-selector').onchange = function() {
			
			    if (canvas.freeDrawingBrush) {
			      canvas.freeDrawingBrush.color = drawingColorEl.value;
			      canvas.freeDrawingBrush.width = parseInt(drawingLineWidthEl.value, 10) || 1;
			      canvas.freeDrawingBrush.shadowBlur = parseInt(drawingShadowWidth.value, 10) || 0;
			    }
			};
			
			drawingColorEl.onchange = function() {
			    canvas.freeDrawingBrush.color = this.value;
			};
			  
			drawingLineWidthEl.onchange = function() {
			    canvas.freeDrawingBrush.width = parseInt(this.value, 10) || 1;
			    this.previousSibling.innerHTML = this.value;
			};
			
			if (canvas.freeDrawingBrush) {
			    canvas.freeDrawingBrush.color = drawingColorEl.value;
			    canvas.freeDrawingBrush.width = parseInt(drawingLineWidthEl.value, 10) || 1;
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
		})();
	});
