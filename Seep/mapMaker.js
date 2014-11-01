var map;
var addSpringFromCoords;
var editorWidget;

var generateUUID = function () {
	var d = Date.now();
	var uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function (c) {
		var r = (d + Math.random() * 16) % 16 | 0;
                d = Math.floor(d / 16);
                return (c == 'x' ? r : (r & 0x7 | 0x8)).toString(16);
         });
        
        return uuid;
};
        
// Panojs
//PanoJS.MSG_BEYOND_MIN_ZOOM = null;
//PanoJS.MSG_BEYOND_MAX_ZOOM = null;
var viewer = null;

var createViewer = function ( viewer, dom_id, url, prefix, w, h ) {
	if (viewer) return;
  
	var MY_URL      = url;
	var MY_PREFIX   = prefix;
	var MY_TILESIZE = 256;
	var MY_WIDTH    = w;
	var MY_HEIGHT   = h;
	var myPyramid = new ImgcnvPyramid( MY_WIDTH, MY_HEIGHT, MY_TILESIZE);
    
	var myProvider = new PanoJS.TileUrlProvider('','','');
        
	myProvider.assembleUrl = function(xIndex, yIndex, zoom) {
        	return MY_URL + '/' + MY_PREFIX + myPyramid.tile_filename( zoom, xIndex, yIndex );
        }    
    
        viewer = new PanoJS(dom_id, {
        	tileUrlProvider : myProvider,
        	initialZoom	: 0,
        	tileSize        : myPyramid.tilesize,
        	tileExtension	: "jpg",
        	maxZoom         : myPyramid.getMaxLevel(),
        	imageWidth      : myPyramid.width,
        	imageHeight     : myPyramid.height,
        	blankTile       : '/panojs/images/blank.gif',
        	loadingTile     : '/panojs/images/progress.gif'
        });

        Ext.EventManager.addListener( window, 'resize', callback(viewer, viewer.resize) );
        viewer.init();
};

var imagePan = function () {
	dialog_image.imagePan();
};

        //console.log(location.href.substring(0, location.href.lastIndexOf("/") + 1));

         // addNew
         // bring up the form editor
         // add a point to newfoundFeatureLayer
         // add to the collection

/*         "esri/config",

        "dojo/_base/event", 
        "dojo/parser", 
        "dijit/registry",

        "dijit/form/Button"
 */
        require([
            "esri/map",
            "esri/dijit/Scalebar",
            "esri/layers/FeatureLayer",
            "esri/dijit/PopupTemplate",
            "esri/geometry/Point",
            "esri/graphic",
            "esri/renderers/SimpleRenderer",
            "esri/renderers/UniqueValueRenderer",
            "dojo/on",
            "dojo",
            "esri/dijit/Legend",
            "dojo/_base/array",
            "dojo/_base/event", 
            "dojo/parser",
            "dojo/dom",
            'dojo/dom-style',
            "dojo/_base/lang",
            "dijit/registry",
            "esri/layers/CodedValueDomain",
            "esri/tasks/GeometryService",
            "esri/toolbars/edit",
            "esri/toolbars/draw",
            "esri/dijit/editing/TemplatePicker",
            "esri/layers/FeatureType",
            "esri/symbols/SimpleMarkerSymbol",
            "esri/Color",
            "esri/InfoTemplate",
            "esri/dijit/Popup",
            "esri/urlUtils",
            "dijit/layout/BorderContainer",
            "dijit/layout/ContentPane",
            "dijit/layout/AccordionContainer",
            "dojox/layout/FloatingPane",
            "dojox/layout/Dock",
            "dijit/form/Button",
            "dojo/domReady!"
        ],
            function (Map,
                Scalebar,
                FeatureLayer,
                PopupTemplate,
                Point,
                Graphic,
                simpleRenderer,
                uniqueValueRenderer,
                on,
                dojo,
                Legend,
                arrayUtils,
                event,
                parser,
                dom,
                domStyle,
                lang,
                registry,
//                Editor,
                CodedValueDomain,
                GeometryService,
                Edit,
                Draw,
                TemplatePicker,
                FeatureType,
                SimpleMarkerSymbol,
                Color,
                InfoTemplate,
//                FeatureTemplate,
                Popup,
                urlUtils) {
                parser.parse();

                
                map = new Map("map", {
                    basemap: "topo",
                    center: [-111.93, 34.17],
                    zoom: 7
                });
                
                map.on("zoom-end", function (evt) {
                		if (evt.level>=11) {
                			domStyle.set(dom.byId("zoomedIn"), "display", "block");
                		} else {
                			domStyle.set(dom.byId("zoomedIn"), "display", "none");
                		}
                });

                map.on("layers-add-result", function (evt) {
                    // add the legend, first filter out the layers not needed
                    var filteredLayerInfo = arrayUtils.filter(evt.layers, function (layer, index) {
                        return layer.layer.name;
                    });

                    var layerInfo = arrayUtils.map(filteredLayerInfo, function (layer, index) {

                        if (layer.layer.name == "DLCC Springs") {
                            return {
                                layer: layer.layer,
                                title: "Published Springs"
                            };
                        } else if (layer.layer.name == "springsLCC.DBO.DLCC_Boundary") {
                            return {
                                layer: layer.layer,
                                title: "Published Springs Boundary"
                            };
                        } else if (layer.layer.name == "HideAndSeep.DBO.Images" && layer.layer.id==2) {
                            return {
                                layer: layer.layer,
                                title: "Hide & Seep Photos"
                            };
                        } else if (layer.layer.name == "HideAndSeep.DBO.AnnotatedSprings" && layer.layer.id==1) {
                            return {
                                layer: layer.layer,
                                title: "Hide & Seep Springs"
                            };
                        } else {
                            return {
                                layer: null
                            };
                        }
                    });

                    if (layerInfo.length > 0) {
                        var legendDijit = new Legend({
                            map: map,
                            layerInfos: layerInfo
                        }, "legendDiv");
                        legendDijit.startup();
                    }

                    // create editor
                    var currentLayer = null;

                     var filteredTemplateLayers = arrayUtils.filter(evt.layers, function (layer, index) {
                        if (layer.layer.id=="5") {
                        	return layer.layer;
                        }
                    });

                    var templateLayers = arrayUtils.map(filteredTemplateLayers, function (layer, index) {
                    	layer.layer.name = "Hide & Seep Springs";
                        return layer.layer;
                    });
                   
                   console.log(templateLayers);

                   var editToolbar = new Edit(map);
                   editToolbar.on("deactivate", function(evt) {
                   		   currentLayer.applyEdits(null, [evt.graphic], null);
                   });
                   
                   arrayUtils.forEach(templateLayers, function(layer) {
                   		   var editingEnabled = false;
                   		   layer.on("dbl-click", function(evt) {
                   		   		   event.stop(evt);
                   		   		   if (editingEnabled === false) {
                   		   		   	   editingEnabled = true;
                   		   		   	   editToolbar.activate(Edit.EDIT_VERTICES , evt.graphic);
                   		   		   } else {
                   		   		   	   currentLayer = this;
                   		   		   	   editToolbar.deactivate();
                   		   		   	   editingEnabled = false;
                   		   		   }
                   		   });
                   		   
                   		   layer.on("click", function(evt) {
                   		   		   event.stop(evt);
                   		   		   if (evt.ctrlKey === true || evt.metaKey === true) {  //delete feature if ctrl key is depressed
                   		   		   	   layer.applyEdits(null,null,[evt.graphic]);
                   		   		   	   currentLayer = this;
                   		   		   	   editToolbar.deactivate();
                   		   		   	   editingEnabled=false;
                   		   		   }
                   		   });
                   });
                   
                   var templatePicker = new TemplatePicker({
                   		   featureLayers: templateLayers,
                   		   rows: "auto",
                   		   columns: 3,
                   		   grouping: true,
                   		   style: "height: auto; overflow: auto;"
                   }, "templatePickerDiv");
                   
                   templatePicker.startup();
                   
                   var drawToolbar = new Draw(map);
                   
                   var selectedTemplate;
                   templatePicker.on("selection-change", function() {
                   		   if( templatePicker.getSelected() ) {
                   		   	   selectedTemplate = templatePicker.getSelected();
                   		   }
                   		   switch (selectedTemplate.featureLayer.geometryType) {
                   		   case "esriGeometryPoint":
                   		   	   drawToolbar.activate(Draw.POINT);
                   		   	   break;
                    		   }
                   });
                   
                   drawToolbar.on("draw-end", function(evt) {
                   		   drawToolbar.deactivate();
                   		   editToolbar.deactivate();
                   		   var newAttributes = lang.mixin({}, selectedTemplate.template.prototype.attributes);
                   		   
                   		   layers[0] = addSeepObject(layers[0]);
    	
                   		   indx = layers[0].objects.length-1;
    	
                   		   layers[0].objects[indx].attributes[0].value = generateUUID();
    	
                   		   dialog_seepMain.seepSelected(evt.geometry, newAttributes, selectedTemplate.featureLayer);
                   		   
                   		   dialog_seepMain.show();                   		   
                   });
                   
                  domStyle.set(dom.byId("zoomedIn"), "display", "none");
                  
                   on(dom.byId('btnZoom'), 'click', function () {
                   		   	map.setZoom(11);
 //                  		   	domStyle.set(dom.byId("zoomedIn"), "display", "block");
                   });
                   
                   domStyle.set(dom.byId("loading"), "display", "none");

                 });
                
                /* ----------------------------------------------------------------------------- */
                
                //load feature layers
                seepBoundaryMapLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/DLCC/DLCC_Boundary/MapServer/0", {
                    id: '3',
                    visible: true,
                    infoTemplate: new InfoTemplate("Boundary", "Desert Landscape Conservation Cooperative Springs Distribution"),
                    outFields: ["area_names"]
                });

                // get the springs
                seepExistingSpringsLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/DLCC/DLCC_Springs/MapServer/0", {
                    id: '4',
                    visible: true,
                    infoTemplate: new InfoTemplate("${NAME}", "<b>Elevation</b>: ${ElevM} m.<br>" +
                        "<b>Location</b>: ${County}<br><b>Land Unit</b>: ${LandUnit}<br>" +
                        "<b>Land Detail</b>: ${LandDetail}<br><b>Latitude</b>: ${Latitude}<br><b>Longitude</b>: ${Longitude}"),
                    outFields: [
                    "NAME", "ElevM", "County", "LandUnit", "LandDetail", "Latitude", "Longitude"
                ]
                });

                urlUtils.addProxyRule({
                    urlPrefix: "arcgis.springsdata.org/arcgis/rest/services/Global/HideAndSeepWB",
                    proxyUrl: "http://overtexplorations.com/proxy/proxy.php"
                });

                seepMapLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideAndSeepWB/MapServer/1", {
                    id: '1',
                    visible: true
                });
 
                 seepImagesMapLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideAndSeepWB/MapServer/0", {
                    id: '2',
                    visible: true
                });
                 
                seepFeatureLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideAndSeepWB/FeatureServer/1", {
                    id: '5',
                    mode: FeatureLayer.MODE_SNAPSHOT,
                    editable: true,
                    visible: true,
                    outFields: [ "*" ]
                });
                
                seepImagesFeatureLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideAndSeepWB/FeatureServer/0", {
                    id: '6',
                    mode: FeatureLayer.MODE_SNAPSHOT,
                    editable: true,
                    visible: true,
                    outFields: [ "*" ]
                });
 
                var layerScales = [
                    {
                        min: 288895.2884,
                        max: 0,
                        level: 11
                    }, // Existing Springs and annotated
                    {
                        min: 4513.98888,
                        max: 0,
                        level: 17
                    }, // Images
                    {
                        min: 36978596.91,
                        max: 288896,
                        level: 4
                    } // Boundary
                ];

                /////////////////////////////// boundary ////////////////////////
                
                seepBoundaryMapLayer.setMaxScale(layerScales[2].max);
                seepBoundaryMapLayer.setMinScale(layerScales[2].min);

                /////////////////////////////// Existing Springs ////////////////////
                
                seepExistingSpringsLayer.setMaxScale(layerScales[0].max);
                seepExistingSpringsLayer.setMinScale(layerScales[0].min);

                /////////////////////////////// Seep images ///////////////////////
                
                seepImagesFeatureLayer.setMaxScale(layerScales[1].max);
                seepImagesFeatureLayer.setMinScale(layerScales[1].min);

                seepImagesMapLayer.setMaxScale(layerScales[1].max);
                seepImagesMapLayer.setMinScale(layerScales[1].min);
 
                /////////////////////////////// Seep ////////////////////////////////
                
                seepMapLayer.setMaxScale(layerScales[0].max);
                seepMapLayer.setMinScale(layerScales[0].min);

                seepFeatureLayer.setMaxScale(layerScales[0].max);
                seepFeatureLayer.setMinScale(layerScales[0].min);
            	
            	var createSymbol = function(path, color){
            		var markerSymbol = new SimpleMarkerSymbol();
            		markerSymbol.setPath(path);
            		markerSymbol.setColor(new Color(color));
            		markerSymbol.setOutline(null);
            		return markerSymbol;
            	};
                
                var iconPaths = [ { name: "unknown", color: "#7B8C1E", path: "M16,1.466C7.973,1.466,1.466,7.973,1.466,16c0,8.027,6.507,14.534,14.534,14.534c8.027,0,14.534-6.507,14.534-14.534C30.534,7.973,24.027,1.466,16,1.466z M17.328,24.371h-2.707v-2.596h2.707V24.371zM17.328,19.003v0.858h-2.707v-1.057c0-3.19,3.63-3.696,3.63-5.963c0-1.034-0.924-1.826-2.134-1.826c-1.254,0-2.354,0.924-2.354,0.924l-1.541-1.915c0,0,1.519-1.584,4.137-1.584c2.487,0,4.796,1.54,4.796,4.136C21.156,16.208,17.328,16.627,17.328,19.003z"},
                	{ name: "existing", color: "#A3B443", path: "M22.727,18.242L4.792,27.208l8.966-8.966l-4.483-4.484l17.933-8.966l-8.966,8.966L22.727,18.242z"},
                	{ name: "new", color: "#EDF8AB", path: "M24.485,2c0,8-18,4-18,20c0,6,2,8,2,8h2c0,0-3-2-3-8c0-4,9-8,9-8s-7.981,4.328-7.981,8.436C21.239,24.431,28.288,9.606,24.485,2z"},
                	{ name: "photo", color: "#A29B40", path: "M15.318,7.677c0.071-0.029,0.148-0.046,0.229-0.046h11.949c-2.533-3.915-6.938-6.506-11.949-6.506c-5.017,0-9.428,2.598-11.959,6.522l4.291,7.431C8.018,11.041,11.274,7.796,15.318,7.677zM28.196,8.84h-8.579c2.165,1.357,3.605,3.763,3.605,6.506c0,1.321-0.334,2.564-0.921,3.649c-0.012,0.071-0.035,0.142-0.073,0.209l-5.973,10.347c7.526-0.368,13.514-6.587,13.514-14.205C29.77,13.002,29.201,10.791,28.196,8.84zM15.547,23.022c-2.761,0-5.181-1.458-6.533-3.646c-0.058-0.046-0.109-0.103-0.149-0.171L2.89,8.855c-1,1.946-1.565,4.153-1.565,6.492c0,7.624,5.999,13.846,13.534,14.205l4.287-7.425C18.073,22.698,16.848,23.022,15.547,23.022zM9.08,15.347c0,1.788,0.723,3.401,1.894,4.573c1.172,1.172,2.785,1.895,4.573,1.895c1.788,0,3.401-0.723,4.573-1.895s1.895-2.785,1.895-4.573c0-1.788-0.723-3.4-1.895-4.573c-1.172-1.171-2.785-1.894-4.573-1.894c-1.788,0-3.401,0.723-4.573,1.894C9.803,11.946,9.081,13.559,9.08,15.347z"},
                	{ name: "video", color: "#A27B40", path: "M27.188,4.875v1.094h-4.5V4.875H8.062v1.094h-4.5V4.875h-1v21.25h1v-1.094h4.5v1.094h14.625v-1.094h4.5v1.094h1.25V4.875H27.188zM8.062,23.719h-4.5v-3.125h4.5V23.719zM8.062,19.281h-4.5v-3.125h4.5V19.281zM8.062,14.844h-4.5v-3.125h4.5V14.844zM8.062,10.406h-4.5V7.281h4.5V10.406zM11.247,20.59V9.754l9.382,5.418L11.247,20.59zM27.188,23.719h-4.5v-3.125h4.5V23.719zM27.188,19.281h-4.5v-3.125h4.5V19.281zM27.188,14.844h-4.5v-3.125h4.5V14.844zM27.188,10.406h-4.5V7.281h4.5V10.406z"},
                	{ name: "arrow", color: "#000000", path: "M8.5454545,55.090909C31.090909,11.454545,31.272727,3.6363637,31.272727,3.6363637,36.07112,20.391033,41.302739,36.712034,50.909091,54.909091,37.171372,38.60001,36.666666,37.393939,30,30.727272,22.727272,37.636363,22.478498,39.491067,8.5454545,55.090909z"}
                ];
                
                var uniqueRenderer = new uniqueValueRenderer(null, "TYPE");
                
                arrayUtils.forEach(iconPaths, function(iconPath) {
                	var symbol = createSymbol(iconPath.path, iconPath.color);
                	
                	symbol.size = 30;
                	
                	var renderer = null;
                	
                	switch (iconPath.name) {
                		case "photo":
                			renderer = new simpleRenderer(symbol);
                			seepImagesMapLayer.setRenderer(renderer);
                			seepImagesFeatureLayer.setRenderer(renderer);
                			break;
                		case "arrow":
                			// do nothing yet
                			break;
                		case "video":
                			// do nothing yet
                			break;
                		// seep features
                		case "unknown":
                			uniqueRenderer.addValue({
                					value: 0,
                					symbol: symbol,
                					label: "Unknown"
                			});
                			break
                		case "existing":
                 			uniqueRenderer.addValue({
                					value: 1,
                					symbol: symbol,
                					label: "Existing"
                			});
                			break
                		case "new":
                			uniqueRenderer.addValue({
                					value: 2,
                					symbol: symbol,
                					label: "New"
                			});
                			break
                	}
            	});
            	
            	seepMapLayer.setRenderer(uniqueRenderer);
            	seepFeatureLayer.setRenderer(uniqueRenderer);

                map.addLayers([seepFeatureLayer, seepBoundaryMapLayer, seepExistingSpringsLayer, seepMapLayer, seepImagesMapLayer, seepImagesFeatureLayer]);

                // scalebar
                var scalebar = new Scalebar({
                    map: map,
                    // "dual" displays both miles and kilmometers
                    // "english" is the default, which displays miles
                    // use "metric" for kilometers
                    scalebarUnit: "english"
                });

                // north arrow
                dojo.byId("north").innerHTML = "<img id='img' src='images/north-arrow.gif' />";
                 
});