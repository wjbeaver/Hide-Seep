// var map
// var addSpringFromCoords;
// var editorWidget;
// var springDeleted;

appConfig.generateUUID = function () {
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
	appConfig.dialog_image.imagePan();
};

require([
            "esri/map",
            "esri/dijit/Scalebar",
            "esri/dijit/OverviewMap",
            "esri/dijit/BasemapToggle",
            "esri/layers/FeatureLayer",
            "esri/dijit/PopupTemplate",
            "esri/geometry/Point",
            "esri/geometry/Extent",
			"esri/tasks/query",
			"esri/dijit/AttributeInspector",
			"dojo/dom-construct",
			"dijit/form/Button",
			"esri/dijit/InfoWindow",
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
            "dojo/dom-class",
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
            'formSeepApp/formSeep/attributeBar',
//            "dojox/layout/Dock",
            "esri/SpatialReference",
            "dojo/domReady!"
        ],
            function (Map,
                Scalebar,
                OverviewMap,
                BasemapToggle,
                FeatureLayer,
                PopupTemplate,
                Point,
                Extent,
                Query,
                AttributeInspector,
                domConstruct,
                Button,
                InfoWindow,
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
                domClass,
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
                urlUtils,
                SpatialReference) {
                parser.parse();

                map = new Map("map", {
                    basemap: "topo",
                    center: [-111.93, 34.17],
                    zoom: 7,
                    SpatialReference: new SpatialReference({wkid:3857}),
                    barShow: false
//                    infoWindow: infoWindow
                });
                
                appConfig.map = map;

                map.infoWindow.resize(300,300);
                map.infoWindow.set("anchor", "top");
                
                domClass.add(map.infoWindow.domNode, "popSeep");
                
                appConfig.springDeleted = function () {
                    var bar = dijit.byId('seepAttributes').attributeBarNode;
                    bar.hide();
                };
                
                appConfig.switchToAdminLayers = function() {
                    if (appConfig.adminLayers.length==0) {
                            newLayer = {
                                layer: null,
                                id: "",
                                position: 0
                            }
                        
                        var layer = null;
                        
                        // load admin layers
                        newLayer.layer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/FeatureServer/1", {
                            id: '5',
                            mode: FeatureLayer.MODE_SNAPSHOT,
                            editable: true,
                            visible: true,
                            outFields: [ "*" ]
                        });
                        
                        newLayer.id = "5";
                        newLayer.position = 3;
                        
                        layer = appConfig.map.getLayer(newLayer.id);
                        
                        // max and min
                        newLayer.layer.setMaxScale(layer.maxScale)
                        newLayer.layer.setMinScale(layer.minScale)
                        
                        // renderer
                        newLayer.layer.setRenderer(layer.renderer);
                        
                        appConfig.adminLayers[0] = newLayer;
                        
                        newLayer.layer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/MapServer/0", {
                            id: '2',
                            visible: true
                        });
                        
                        newLayer.id = "2";
                        newLayer.position = 4;
                        
                        layer = appConfig.map.getLayer(newLayer.id);
                        
                        // max and min
                        newLayer.layer.setMaxScale(layer.maxScale)
                        newLayer.layer.setMinScale(layer.minScale)
                        
                        // renderer
                        newLayer.layer.setRenderer(layer.renderer);
                        
                        appConfig.adminLayers[1] = newLayer;
             
                        newLayer.layer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/FeatureServer/0", {
                            id: '6',
                            mode: FeatureLayer.MODE_SNAPSHOT,
                            editable: true,
                            visible: true,
                            infoTemplate: infoTemplate,
                            outFields: [ "*" ]
                        });
                        
                        newLayer.id = "6";
                        newLayer.position = 5;
                        
                        layer = appConfig.map.getLayer(newLayer.id);
                        
                        // max and min
                        newLayer.layer.setMaxScale(layer.maxScale)
                        newLayer.layer.setMinScale(layer.minScale)
                        
                        // renderer
                        newLayer.layer.setRenderer(layer.renderer);
                        
                        // infoTemplate
                        newLayer.layer.setInfoTemplate(layer.infoTemplate);
                        
                        appConfig.adminLayers[2] = newLayer;
                        
                        newLayer.layer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/MapServer/1", {
                            id: '1',
                            editable: true,
                            visible: true,
                            outFields: [ "*" ]
                        });
                        
                        newLayer.id = "1";
                        newLayer.position = 6;
                         
                        layer = appConfig.map.getLayer(newLayer.id);
                        
                        // max and min
                        newLayer.layer.setMaxScale(layer.maxScale)
                        newLayer.layer.setMinScale(layer.minScale)
                        
                        // renderer
                        newLayer.layer.setRenderer(layer.renderer);
                        
                        appConfig.adminLayers[3] = newLayer;
                    }
                    
                    // remove each user layer and add a admin layer
                    for (n=0;n<appConfig.adminLayers.length;n++) {
                        appConfig.map.removeLayer(map.getLayer(appConfig.adminLayers[n].id));
                        appConfig.map.addLayer(appConfig.adminLayers[n].layer, appConfig.adminLayers[n].position);
                    }
                };
                
                appConfig.switchToUserLayers = function() {
                    // remove each admin layer and add a user layer
                    for (n=0;n<appConfig.userLayers.length;n++) {
                        appConfig.map.removeLayer(map.getLayer(appConfig.userLayers[n].id));
                        appConfig.map.addLayer(appConfig.userLayers[n].layer, appConfig.userLayers[n].position);
                    }
                };

                var overviewMap = new OverviewMap({
                    map: map,
                    attachTo: "bottom-left",
                    color:" #D84E13",
                    opacity: .40,
                    visible: true
                });
        
                overviewMap.startup();
                
                var toggle = new BasemapToggle({
                    map: map,
                    basemap: "satellite"
                }, "BasemapToggle");
                
                toggle.startup();

                map.on("layers-add-result", function (evt) {
                    // add the legend, first filter out the layers not needed
                    var filteredLayerInfo = arrayUtils.filter(evt.layers, function (layer, index) {
                        return layer.layer.name;
                    });

                    var layerInfo = arrayUtils.map(filteredLayerInfo, function (layer, index) {

                        if (layer.layer.name == "DLCC Springs") {
                            return {
                                layer: layer.layer,
                                title: "Desert LCC Published Springs"
                            };
                        } else if (layer.layer.name == "Southern Rockies LCC Springs") {
                            return {
                                layer: layer.layer,
                                title: "Southern Rockies LCC Published Springs"
                            };
                        } else if (layer.layer.name == "springsLCC.DBO.DLCC_US") {
                            return {
                                layer: layer.layer,
                                title: "Desert LCC Published Springs Boundary"
                            };
                        } else if (layer.layer.name == "Southern Rockies LCC Boundary") {
                            return {
                                layer: layer.layer,
                                title: "Southern Rockies LCC Published Springs Boundary"
                            };
                        } else if (layer.layer.name == "Pictures" && layer.layer.id==2) {
                            return {
                                layer: layer.layer,
                                title: "Hide & Seep Photos"
                            };
                        } else if (layer.layer.name == "Annotated Springs" && layer.layer.id==1) {
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
                   		   if ( templatePicker.getSelected() ) {
                   		   	   selectedTemplate = templatePicker.getSelected();
                   		   	   
                               switch (selectedTemplate.featureLayer.geometryType) {
                                   case "esriGeometryPoint":
                                       drawToolbar.activate(Draw.POINT);
                                       break;
                                   }
                            } else {
                                drawToolbar.deactivate();
                            }
                   });
                   
                   drawToolbar.on("draw-end", function(evt) {
//                   		   drawToolbar.deactivate();
                            var newAttributes = lang.mixin({}, selectedTemplate.template.prototype.attributes);

                            appConfig.dialog_seepMain.clearLayers();
                            
                            appConfig.layers[0] = addSeepObject(appConfig.layers[0]);

                            appConfig.layers[0].objects[0].attributes[0].value = appConfig.generateUUID();

                            appConfig.dialog_seepMain.seepSelected(evt.geometry, newAttributes, selectedTemplate);

                            appConfig.dialog_seepMain.show();                 		                    		   
                   });
                   
                  domStyle.set(dom.byId("zoomedIn"), "display", "none");
                  
                   on(dom.byId('btnZoom'), 'click', function () {
                   		   	map.setZoom(11);
                    });
                   
                   domStyle.set(dom.byId("loading"), "display", "none");

                 });
                
                /* ----------------------------------------------------------------------------- */
                
                //load feature layers
                seepBoundaryMapLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/DLCC_SRLCC/DLCC_SRLCC_Boundary/MapServer/0", {
                    id: '3',
                    visible: true,
                    infoTemplate: new InfoTemplate("Boundary", "Desert Landscape Conservation Cooperative Springs Distribution"),
                    outFields: ["area_names"]
                });

                seepBoundary2MapLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/DLCC_SRLCC/DLCC_SRLCC_Boundary/MapServer/1", {
                    id: '9',
                    visible: true,
                    infoTemplate: new InfoTemplate("Boundary", "Southern Rockies Landscape Conservation Cooperative Springs Distribution"),
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
                    "NAME", "ElevM", "County", "LandUnit", "Latitude", "Longitude"
                    ]
                });

                // get the springs
                seep2ExistingSpringsLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/SRLCC/SRLCC_Springs/MapServer/0", {
                    id: '10',
                    visible: true,
                    infoTemplate: new InfoTemplate("${NAME}", "<b>Elevation</b>: ${ElevM} m.<br>" +
                        "<b>Location</b>: ${County}<br><b>Land Unit</b>: ${LandUnit}<br>" +
                        "<b>Land Detail</b>: ${LandDetail}<br><b>Latitude</b>: ${Latitude}<br><b>Longitude</b>: ${Longitude}"),
                    outFields: [
                    "NAME", "ElevM", "County", "LandUnit", "Latitude", "Longitude"
                    ]
                });

                urlUtils.addProxyRule({
                    urlPrefix: "arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep",
                    proxyUrl: "http://overtexplorations.com/proxy/proxy.php"
                });
                
                newLayer = {
                    layer: null,
                    id: "",
                    position: 0
                }
                
                seepFeatureLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/FeatureServer/1", {
                    id: '5',
                    mode: FeatureLayer.MODE_SNAPSHOT,
                    editable: true,
                    visible: true,
                    outFields: [ "*" ]
                });
                
                seepFeatureLayer.setDefinitionExpression("UPLOADID_PK='X'");
                
                newLayer.layer = seepFeatureLayer;
                newLayer.id = "5";
                newLayer.position = 3;
                appConfig.userLayers[0] = newLayer;
                
                seepImagesMapLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/MapServer/0", {
                    id: '2',
                    visible: true
                });
                
                seepImagesMapLayer.setDefinitionExpression("UPLOADID_FK='X'");
                
                newLayer.layer = seepImagesMapLayer;
                newLayer.id = "2";
                newLayer.position = 4;
                appConfig.userLayers[1] = newLayer;
             
                infoTemplate = new InfoTemplate();
                infoTemplate.setTitle("${Title}");
                infoTemplate.setContent("<b>Description</b>: ${Description}<br>" +
                        '<a href="${IMAGE}" target="_blank"><img src="${IMAGE:small}" /></a><br>' +
                        "<b>Time Zone</b>: ${TimeZoneName}<br><b>UTC</b>: ${UTC}<br>" + 
                        "${Altitude:fill}${Orientation:fill}");

                seepImagesFeatureLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/FeatureServer/0", {
                    id: '6',
                    mode: FeatureLayer.MODE_SNAPSHOT,
                    editable: true,
                    visible: true,
                    infoTemplate: infoTemplate,
                    outFields: [ "*" ]
                });
                
                seepImagesFeatureLayer.setDefinitionExpression("UPLOADID_FK='X'");
                
                newLayer.layer = seepImagesFeatureLayer;
                newLayer.id = "6";
                newLayer.position = 5;
                appConfig.userLayers[2] = newLayer;

                seepMapLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/MapServer/1", {
                    id: '1',
                    editable: true,
                    visible: true,
                    outFields: [ "*" ]
                });
                
                seepMapLayer.setDefinitionExpression("UPLOADID_PK='X'");
                
                newLayer.layer = seepMapLayer;
                newLayer.id = "1";
                newLayer.position = 6;
                appConfig.userLayers[3] = newLayer;

                fill = function (value, key, data) {
                  var results = "";
                  
                    switch (key) {
                        case "Altitude":
                            if (data.Altitude!=null) {
                                results = "<b>Altitude</b>: " + data.Altitude + " m.";
                            }
                            break;
                        case "Orientation":
                            if (data.Orientation!=null) {
                                results = "<b>Orientation</b>: " + data.Orientation + "&deg;";
                            }
                            break;
                    }
                  return results;
                };

                small = function (value, key, data) {
                  var results = "";

                  var parts = data.IMAGE.split("/");
                  var num = parts.length-1;
                  var name = parts[num];
                  var names = name.split(".");
                  names[0] = "sized_"+names[0]+"/"+names[0]+"_small";
                  name = names.join(".");
                  parts[num] = name;
                  results = parts.join("/");
                  
                  return results;
                };

                seepNamesFeatureLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/FeatureServer/2", {
                    id: '7',
                    mode: FeatureLayer.MODE_SNAPSHOT,
                    editable: true,
                    visible: false,
                    outFields: [ "*" ]
                });

                seepFeatureNameFeatureLayer = new FeatureLayer("https://arcgis.springsdata.org/arcgis/rest/services/Global/HideSeep/FeatureServer/3", {
                    id: '8',
                    mode: FeatureLayer.MODE_SNAPSHOT,
                    editable: true,
                    visible: false,
                    outFields: [ "*" ]
                });
                
                var layerScales = [
                    {
                        min: 288895.2884,
                        max: 0,
                        level: 11
                    }, // Existing Springs and annotated
                    {
                        min: 18055.955520,
                        max: 0,
                        level: 15
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
                seepBoundary2MapLayer.setMaxScale(layerScales[2].max);
                seepBoundary2MapLayer.setMinScale(layerScales[2].min);

                /////////////////////////////// Existing Springs ////////////////////
                
                seepExistingSpringsLayer.setMaxScale(layerScales[0].max);
                seepExistingSpringsLayer.setMinScale(layerScales[0].min);
                seep2ExistingSpringsLayer.setMaxScale(layerScales[0].max);
                seep2ExistingSpringsLayer.setMinScale(layerScales[0].min);

                /////////////////////////////// Seep images ///////////////////////
                
                seepImagesFeatureLayer.setMaxScale(layerScales[1].max);
                seepImagesFeatureLayer.setMinScale(layerScales[1].min);

                seepImagesMapLayer.setMaxScale(layerScales[1].max);
                seepImagesMapLayer.setMinScale(layerScales[1].min);
 
                /////////////////////////////// Seep ////////////////////////////////
                
                seepMapLayer.setMaxScale(layerScales[0].max);
                seepMapLayer.setMinScale(layerScales[2].min);

                seepFeatureLayer.setMaxScale(layerScales[0].max);
                seepFeatureLayer.setMinScale(layerScales[2].min);
            	
            	var createSymbol = function(path, color){
            		var markerSymbol = new SimpleMarkerSymbol();
            		markerSymbol.setPath(path);
            		markerSymbol.setColor(new Color(color));
            		markerSymbol.setOutline(null);
            		return markerSymbol;
            	};
                
                var iconPaths = [ { name: "unknown", color: "#E3485E", path: "M16,1.466C7.973,1.466,1.466,7.973,1.466,16c0,8.027,6.507,14.534,14.534,14.534c8.027,0,14.534-6.507,14.534-14.534C30.534,7.973,24.027,1.466,16,1.466z M17.328,24.371h-2.707v-2.596h2.707V24.371zM17.328,19.003v0.858h-2.707v-1.057c0-3.19,3.63-3.696,3.63-5.963c0-1.034-0.924-1.826-2.134-1.826c-1.254,0-2.354,0.924-2.354,0.924l-1.541-1.915c0,0,1.519-1.584,4.137-1.584c2.487,0,4.796,1.54,4.796,4.136C21.156,16.208,17.328,16.627,17.328,19.003z"},
                	{ name: "existing", color: "#A3B443", path: "M22.727,18.242L4.792,27.208l8.966-8.966l-4.483-4.484l17.933-8.966l-8.966,8.966L22.727,18.242z"},
                	{ name: "new", color: "#526199", path: "M24.485,2c0,8-18,4-18,20c0,6,2,8,2,8h2c0,0-3-2-3-8c0-4,9-8,9-8s-7.981,4.328-7.981,8.436C21.239,24.431,28.288,9.606,24.485,2z"},
                	{ name: "photo", color: "#725f2d", path: "M15.318,7.677c0.071-0.029,0.148-0.046,0.229-0.046h11.949c-2.533-3.915-6.938-6.506-11.949-6.506c-5.017,0-9.428,2.598-11.959,6.522l4.291,7.431C8.018,11.041,11.274,7.796,15.318,7.677zM28.196,8.84h-8.579c2.165,1.357,3.605,3.763,3.605,6.506c0,1.321-0.334,2.564-0.921,3.649c-0.012,0.071-0.035,0.142-0.073,0.209l-5.973,10.347c7.526-0.368,13.514-6.587,13.514-14.205C29.77,13.002,29.201,10.791,28.196,8.84zM15.547,23.022c-2.761,0-5.181-1.458-6.533-3.646c-0.058-0.046-0.109-0.103-0.149-0.171L2.89,8.855c-1,1.946-1.565,4.153-1.565,6.492c0,7.624,5.999,13.846,13.534,14.205l4.287-7.425C18.073,22.698,16.848,23.022,15.547,23.022zM9.08,15.347c0,1.788,0.723,3.401,1.894,4.573c1.172,1.172,2.785,1.895,4.573,1.895c1.788,0,3.401-0.723,4.573-1.895s1.895-2.785,1.895-4.573c0-1.788-0.723-3.4-1.895-4.573c-1.172-1.171-2.785-1.894-4.573-1.894c-1.788,0-3.401,0.723-4.573,1.894C9.803,11.946,9.081,13.559,9.08,15.347z"},
//                	{ name: "photo", color: "#725f2d", path: "m 210.40419,182.70238 c 0,0 -0.36019,0.0247 0,0 4.74532,-0.32606 11.40999,5.61044 11.40999,5.61044 -2.533,-3.915 -6.938,-6.506 -11.949,-6.506 -5.017,0 -9.428,2.598 -11.959,6.522 l 1.44432,0.78646 c 0.5818,-2.22837 2.95339,-3.94115 4.91107,-4.89501 1.95768,-0.95386 4.12062,-1.45839 6.14262,-1.51789 z m 12.10999,6.81944 -2.1274,0.245 c 2.165,1.357 2.46167,4.49799 2.46167,7.24099 0,1.321 -1.04857,6.38187 -3.7793,8.52852 0,0 0.29676,-0.37852 0,0 -0.29676,0.37853 -2.37196,2.13622 -3.82448,2.92987 -1.39491,0.76217 -4.69562,1.7956 -4.69562,1.7956 7.19934,-0.77633 13.5385,-6.61599 13.5385,-14.23399 0.001,-2.344 -0.568,-4.555 -1.573,-6.506 z m -23.3066,14.45383 -0.0438,0.0438 c -4.22956,-4.62845 -3.33377,-10.65257 -1.93748,-14.19116 -0.14798,-1.0839 -1.58314,3.86155 -1.58314,6.20055 0,7.624 5.999,13.846 13.534,14.205 l 1.0523,-0.97063 c -5.16142,0.26512 -9.41497,-3.05951 -11.0219,-5.28754 z m 4.1906,-7.94683 c 0,1.788 0.723,3.401 1.894,4.573 1.172,1.172 2.785,1.895 4.573,1.895 1.788,0 3.401,-0.723 4.573,-1.895 1.172,-1.172 1.895,-2.785 1.895,-4.573 0,-1.788 -0.723,-3.4 -1.895,-4.573 -1.172,-1.171 -2.785,-1.894 -4.573,-1.894 -1.788,0 -3.401,0.723 -4.573,1.894 -1.171,1.172 -1.893,2.785 -1.894,4.573 z"},
                	{ name: "video", color: "#A27B40", path: "M27.188,4.875v1.094h-4.5V4.875H8.062v1.094h-4.5V4.875h-1v21.25h1v-1.094h4.5v1.094h14.625v-1.094h4.5v1.094h1.25V4.875H27.188zM8.062,23.719h-4.5v-3.125h4.5V23.719zM8.062,19.281h-4.5v-3.125h4.5V19.281zM8.062,14.844h-4.5v-3.125h4.5V14.844zM8.062,10.406h-4.5V7.281h4.5V10.406zM11.247,20.59V9.754l9.382,5.418L11.247,20.59zM27.188,23.719h-4.5v-3.125h4.5V23.719zM27.188,19.281h-4.5v-3.125h4.5V19.281zM27.188,14.844h-4.5v-3.125h4.5V14.844zM27.188,10.406h-4.5V7.281h4.5V10.406z"},
                	{ name: "arrow", color: "#000000", path: "M8.5454545,55.090909C31.090909,11.454545,31.272727,3.6363637,31.272727,3.6363637,36.07112,20.391033,41.302739,36.712034,50.909091,54.909091,37.171372,38.60001,36.666666,37.393939,30,30.727272,22.727272,37.636363,22.478498,39.491067,8.5454545,55.090909z"}
                ];
                
                uniqueSpringsRenderer = new uniqueValueRenderer(null, "TYPE");
                uniqueSpringsRendererSmall = new uniqueValueRenderer(null, "TYPE");
                var uniqueImageRenderer = new uniqueValueRenderer(null, "TYPE");
                
                arrayUtils.forEach(iconPaths, function(iconPath) {
                	var symbol = createSymbol(iconPath.path, iconPath.color);
                	
                	symbol.size = 30;
                	
                	var renderer = null;
                	
                	switch (iconPath.name) {
                		case "photo":
                			uniqueImageRenderer.addValue({
                					value: 0,
                					symbol: symbol,
                					label: "No Orientation"
                			});
                			break;
                		case "arrow":
                			uniqueImageRenderer.addValue({
                					value: 1,
                					symbol: symbol,
                					label: "Orientation"
                			});
                			break;
                		case "video":
                			// do nothing yet
                			break;
                		// seep features
                		case "unknown":
                			uniqueSpringsRenderer.addValue({
                					value: 0,
                					symbol: symbol,
                					label: "Unknown"
                			});
                			break
                		case "existing":
                 			uniqueSpringsRenderer.addValue({
                					value: 1,
                					symbol: symbol,
                					label: "Existing"
                			});
                			break
                		case "new":
                			uniqueSpringsRenderer.addValue({
                					value: 2,
                					symbol: symbol,
                					label: "New"
                			});
                			break
                	}
            	});
            	
                arrayUtils.forEach(iconPaths, function(iconPath) {
                	var symbol = createSymbol(iconPath.path, iconPath.color);
                	
                	var renderer = null;
                	
                	switch (iconPath.name) {
                		// seep features
                		case "unknown":
                			uniqueSpringsRendererSmall.addValue({
                					value: 0,
                					symbol: symbol,
                					label: "Unknown"
                			});
                			break
                		case "existing":
                 			uniqueSpringsRendererSmall.addValue({
                					value: 1,
                					symbol: symbol,
                					label: "Existing"
                			});
                			break
                		case "new":
                			uniqueSpringsRendererSmall.addValue({
                					value: 2,
                					symbol: symbol,
                					label: "New"
                			});
                			break
                	}
            	});

            	seepMapLayer.setRenderer(uniqueSpringsRenderer);
            	seepFeatureLayer.setRenderer(uniqueSpringsRenderer);
                seepImagesMapLayer.setRenderer(uniqueImageRenderer);
                seepImagesFeatureLayer.setRenderer(uniqueImageRenderer);
                
                userLayers = [seepBoundary2MapLayer, seepBoundaryMapLayer, seepFeatureLayer, seepImagesMapLayer, seepImagesFeatureLayer, 
                    seepMapLayer, seepNamesFeatureLayer, seepFeatureNameFeatureLayer, 
                    seepExistingSpringsLayer, seep2ExistingSpringsLayer];

                map.addLayers(userLayers);

                // scalebar
                var scalebar = new Scalebar({
                    map: map,
                    attachTo: "bottom-center",
                    // "dual" displays both miles and kilmometers
                    // "english" is the default, which displays miles
                    // use "metric" for kilometers
                    scalebarUnit: "english"
                });

                // north arrow
                dojo.byId("north").innerHTML = "<img id='img' src='images/QGISdefaultNArrow.svg' />";
               
                map.on("zoom-end", lang.hitch(this, function (evt) {
                    var mapLayer = map.getLayer("1");
                    var featureLayer = map.getLayer("5");
                    
                    if (evt.level>=15) {
                        mapLayer.setRenderer(uniqueSpringsRendererSmall);
                        featureLayer.setRenderer(uniqueSpringsRendererSmall);
                    } else {
                        mapLayer.setRenderer(uniqueSpringsRenderer);
                        featureLayer.setRenderer(uniqueSpringsRenderer);
                    }
                    
                    if (evt.level>=11) {
                        domStyle.set(dom.byId("zoomedIn"), "display", "block");
                    } else {
                        domStyle.set(dom.byId("zoomedIn"), "display", "none");
                    }
                }));
                
 				var selectQuery = new Query();
				
               map.on("click", function (evt) {
                    var centerPoint = new Point(evt.mapPoint.x,evt.mapPoint.y,evt.mapPoint.spatialReference);
                    var mapWidth = this.extent.getWidth();

                    //Divide width in map units by width in pixels
                    var pixelWidth = mapWidth/this.width;

                    //Calculate a 50 pixel envelope width (25 pixel tolerance on each side)
                    var tolerance = 50 * pixelWidth;

                    //Build tolerance envelope and set it as the query geometry
                    var queryExtent = new Extent(1,1,tolerance,tolerance,evt.mapPoint.spatialReference);
                    selectQuery.geometry = queryExtent.centerAt(centerPoint);
                    var updateFeature;

                    seepFeatureLayer.selectFeatures(selectQuery, FeatureLayer.SELECTION_NEW, function(features) {
                        var bar = dijit.byId('seepAttributes').attributeBarNode;
                        if (features.length > 0) {
                            if (!map.barShow) {
                                // center and maybe select
                                map.centerAt(centerPoint);

                                //store the current feature
                                updateFeature = features[0];
                                dijit.byId('seepAttributes').setID(features);
                                map.barShow = true;
                                bar.show();
                            }
                        } else {
                            bar.hide();
                            map.barShow = false;
                        }
                    });

                    if (map.getZoom()>=15) {
                        seepImagesFeatureLayer.selectFeatures(selectQuery, FeatureLayer.SELECTION_NEW, function(features) {
                            if (features.length > 0) {
                                // display the photos with attributes
                                map.infoWindow.setFeatures(features);
                                map.infoWindow.show(evt.screenPoint);
                            }
                        });
                    }					
                });
});