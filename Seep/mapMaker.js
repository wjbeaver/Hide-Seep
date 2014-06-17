        var map;
        var editorWidget;

         // addNew
         // bring up the form editor
         // add a point to newfoundFeatureLayer
         // add to the collection

        require([
            "esri/map",
            "esri/dijit/Scalebar",
            "esri/layers/FeatureLayer",
            "esri/dijit/PopupTemplate",
            "esri/geometry/Point",
            "esri/graphic",
            "dojo/on",
            "esri/dijit/Legend",
            "dojo/_base/array",
            "dojo/parser",
            "dojo/dom",
            "dojo/_base/lang",
            "dijit/registry",
            "esri/dijit/editing/Editor",
            "esri/layers/CodedValueDomain",
            "esri/tasks/GeometryService",
            "esri/toolbars/edit",
            "esri/dijit/editing/TemplatePicker",
            "esri/layers/FeatureType",
            "esri/graphic",
            "esri/symbols/SimpleMarkerSymbol",
            "esri/Color",
            "esri/InfoTemplate",
            "dijit/layout/BorderContainer",
            "dijit/layout/ContentPane",
            "dijit/layout/AccordionContainer",
            "dojo/domReady!"
        ], function (Map,
            Scalebar,
            FeatureLayer,
            PopupTemplate,
            Point,
            Graphic,
            on,
            Legend,
            arrayUtils,
            parser,
            dom,
            lang,
            registry,
            Editor,
            CodedValueDomain,
            GeometryService,
            Edit,
            TemplatePicker,
            FeatureType,
            Graphic,
            SimpleMarkerSymbol,
            Color,
            InfoTemplate) {
            parser.parse();

            map = new Map("map", {
                basemap: "topo",
                center: [-111.93, 34.17],
                zoom: 7
            });

            map.on("layers-add-result", function (evt) {
                // add the legend
                var layerInfo = arrayUtils.map(evt.layers, function (layer, index) {
                    return {
                        layer: layer.layer,
                        title: layer.layer.name
                    };
                });

                if (layerInfo.length > 0) {
                    var legendDijit = new Legend({
                        map: map,
                        layerInfos: layerInfo
                    }, "legendDiv");
                    legendDijit.startup();
                }

                // add the templatePicker
                var templateLayers = arrayUtils.map(evt.layers, function (result) {
                    return result.layer;
                });

                console.log("template layers", templateLayers);

                var templatePicker = new TemplatePicker({
                    featureLayers: templateLayers,
                    grouping: true,
                    rows: "auto",
                    columns: 3
                }, "templatePickerDiv");

                templatePicker.startup();

                // add the editor
                var featureLayerInfos = arrayUtils.map(evt.layers, function (layer) {
                    return {
                        "featureLayer": layer.layer
                    };
                });

                console.log("feature Layer Infos", featureLayerInfos);

                var settings = {
                    map: map,
                    geometryService: new GeometryService("http://sampleserver3.arcgisonline.com/arcgis/rest/services/Geometry/GeometryServer"),
                    createOptions: null,
                    layerInfos: featureLayerInfos,
                    toolbarVisible: true
                };

                var params = {
                    settings: settings
                };

                editorWidget = new Editor(params, 'editorDiv');

                editorWidget.startup();
            });

            //////////////////////////////////////// Tables

            // create an object collection for contacts
            var seepContactObjectCollection = {
                "layerDefinition": null,
                "featureSet": {
                    "features": []
                }
            };
            seepContactObjectCollection.layerDefinition = {
                "objectIdField": "ObjectID",
                "name": 'Contact',
                "fields": [{
                    "name": "ObjectID",
                    "alias": "ObjectID",
                    "type": "esriFieldTypeOID"
                }, {
                    "name": "CONTACTID_PK",
                    "alias": "Contact ID",
                    "type": "esriFieldTypeString",
                    "editable": false
                }, {
                    "name": "Email",
                    "alias": "Email",
                    "type": "esriFieldTypeString",
                    "editable": true
                }],
                "relationships": []
            };

            // create an object collection for names
            var seepNameObjectCollection = {
                "layerDefinition": null,
                "featureSet": {
                    "features": []
                }
            };
            seepNameObjectCollection.layerDefinition = {
                "objectIdField": "ObjectID",
                "name": 'Name',
                "fields": [{
                    "name": "ObjectID",
                    "alias": "ObjectID",
                    "type": "esriFieldTypeOID"
                }, {
                    "name": "NAMEID_PK",
                    "alias": "Name ID",
                    "type": "esriFieldTypeString",
                    "editable": false
                }, {
                    "name": "Honorific",
                    "alias": "Honorific",
                    "type": "esriFieldTypeString",
                    "editable": true,
                    "domain": new CodedValueDomain({
                        codedValues: ["", "Mr.", "Mrs.", "Ms.", "Dr."],
                        name: "HonorificDomain",
                        type: "CodedValueDomain"
                    })
                 }, {
                    "name": "FirstName",
                    "alias": "First",
                    "type": "esriFieldTypeString",
                    "editable": true
                }, {
                    "name": "MiddleName",
                    "alias": "Middle",
                    "type": "esriFieldTypeString",
                    "editable": true
                }, {
                    "name": "LastName",
                    "alias": "Last",
                    "type": "esriFieldTypeString",
                    "editable": true
                }],
                "relationships": []
            };

            /////////////////// Feature Classes

            // create a feature collection for related images
            var seepImagesFeatureCollection = {
                "layerDefinition": null,
                "featureSet": {
                    "features": [],
                    "geometryType": "esriGeometryPoint",
                    "objectIdField": "ObjectID"
                }
            };
            seepImagesFeatureCollection.layerDefinition = {
                "geometryType": "esriGeometryPoint",
                "objectIdField": "ObjectID",
                "name": 'Images',
                "drawingInfo": {
                    "renderer": {
                        "type": "simple",
                        "symbol": {
                            "type": "esriPMS",
                            "url": "images/spring.png",
                            "contentType": "image/png",
                            "width": 15,
                            "height": 15
                        }
                    }
                },
                "fields": [{
                    "name": "ObjectID",
                    "alias": "ObjectID",
                    "type": "esriFieldTypeOID"
                }, {
                    "name": "IMAGEID_PK",
                    "alias": "Image ID",
                    "type": "esriFieldTypeString",
                    "editable": false
               }, {
                    "name": "UPLOADID_FK",
                    "alias": "Upload ID",
                    "type": "esriFieldTypeString",
                    "editable": false
                }, {
                    "name": "DateTaken",
                    "alias": "Date taken",
                    "type": "esriFieldTypeDate",
                    "editable": false
                 }, {
                    "name": "TimeTaken",
                    "alias": "Time taken",
                    "type": "esriFieldTypeSpring",
                    "editable": false
               }, {
                    "name": "Timestamp",
                    "alias": "Timestamp",
                    "type": "esriFieldTypeInteger",
                    "editable": false
                }, {
                    "name": "DSTOffset",
                    "alias": "Daylight Savings Time Offset",
                    "type": "esriFieldTypeInteger",
                    "editable": false
                }, {
                    "name": "UTCOffset",
                    "alias": "UTC Offset",
                    "type": "esriFieldTypeInteger",
                    "editable": false
                }, {
                    "name": "TimeZoneID",
                    "alias": "Time Zone ID",
                    "type": "esriFieldTypeString",
                    "editable": false
               }, {
                    "name": "TimeZoneName",
                    "alias": "Time Zone Name",
                    "type": "esriFieldTypeString",
                    "editable": false
                }, {
                    "name": "Image",
                    "alias": "File location",
                    "type": "esriFieldTypeRaster",
                    "editable": false
                }, {
                    "name": "PhotographerID_FK",
                    "alias": "Photographer Name",
                    "type": "esriFieldTypeString",
                    "editable": true
                }, {
                    "name": "Title",
                    "alias": "Title",
                    "type": "esriFieldTypeString",
                    "editable": true
                }, {
                    "name": "Description",
                    "alias": "Description",
                    "type": "esriFieldTypeString",
                    "editable": true
                }],
                "relationships": []
            };

            // layer deinitions constrain what is seen
            // featureset
            //create a feature collection for picked springs

            // domains
            var deviceDomain = new CodedValueDomain({
                codedValues: ["Google", "Garmin", "Other", "App"],
                name: "deviceDomain",
                type: "CodedValueDomain"
            });

            var conditionDomain = new CodedValueDomain({
                codedValues: ["Condition1", "Condition2", "Unknown"],
                name: "conditionDomain",
                type: "CodedValueDomain"
            });

            var flowDomain = new CodedValueDomain({
                codedValues: ["Flow1", "Flow2", "Unknown"],
                name: "flowDomain",
                type: "CodedValueDomain"
            });

            var locationDomain = new CodedValueDomain({
                codedValues: ["Yes", "No", "Don't Know"],
                name: "locationDomain",
                type: "CodedValueDomain"
            });

            // create geometries
            var sms1 = new SimpleMarkerSymbol().setStyle(
                SimpleMarkerSymbol.STYLE_CIRCLE).setColor(
                new Color([0, 255, 0]));

            var sms2 = new SimpleMarkerSymbol().setStyle(
                SimpleMarkerSymbol.STYLE_DIAMOND).setColor(
                new Color([0, 0, 255]));

            var sms3 = new SimpleMarkerSymbol().setStyle(
                SimpleMarkerSymbol.STYLE_X).setColor(
                new Color([0, 0, 0]));

            var seepFeatureCollection = {
                "layerDefinition": null,
                "featureSet": {
                    "features": [],
                    "geometryType": "esriGeometryPoint",
                    "objectIdField": "ObjectID",
                    "displayFieldName": "TYPE",
                    "typeTield": "TYPE",
                    "types": [
                        new FeatureType({
                            "id": 10,
                            "name": "New",
                            "domains": {
                                "Device": deviceDomain,
                                "Condition": conditionDomain,
                                "FLow": flowDomain
                            }
                        }),
                        new FeatureType({
                            "id": 20,
                            "name": "Existing",
                            "domains": {
                                "Device": deviceDomain,
                                "Condition": conditionDomain,
                                "FLow": flowDomain,
                                "Location": locationDomain
                            }
                        }),
                        new FeatureType({
                            "id": 30,
                            "name": "Unknown",
                            "domains": {
                                "Device": deviceDomain,
                                "Condition": conditionDomain,
                                "FLow": flowDomain
                            }
                        })
                    ]
                }
            };
            seepFeatureCollection.layerDefinition = {
                "geometryType": "esriGeometryPoint",
                "objectIdField": "ObjectID",
                "name": 'Annotated Springs',
                "drawingInfo": {
                    "renderer": {
                        "type": "uniqueValue",
                        "field1": "TYPE",
                        "uniqueValueInfos": [
                            {
                                "value": "10",
                                "symbol": sms1,
                                "label": "New"
                            },
                            {
                                "value": "20",
                                "symbol": sms2,
                                "label": "Existing"
                            },
                            {
                                "value": "30",
                                "symbol": sms3,
                                "label": "Unknown"
                            }
                        ]
                    }
                },
                "fields": [{
                    "name": "ObjectID",
                    "alias": "ObjectID",
                    "type": "esriFieldTypeOID"
                }, {
                    "name": "UPLOADID_PK",
                    "alias": "Upload ID",
                    "type": "esriFieldTypeString",
                    "editable": false
                }, {
                    "name": "SPRINGID_FK",
                    "alias": "Spring ID",
                    "type": "esriFieldTypeString",
                    "editable": false
               }, {
                    "name": "TYPE",
                    "alias": "Spring Type",
                    "type": "esriFieldTypeLONG",
                    "editable": true
                }, {
                    "name": "Device",
                    "alias": "Device",
                    "type": "esriFieldTypeString",
                    "editable": true
              }, {
                    "name": "Condition",
                    "alias": "Condition",
                    "type": "esriFieldTypeString",
                    "editable": true
                }, {
                    "name": "Flow",
                    "alias": "Flow",
                    "type": "esriFieldTypeString",
                    "editable": true
                }, {
                    "name": "Location",
                    "alias": "Is spring located where shown on the map?",
                    "type": "esriFieldTypeString",
                    "editable": true
               }, {
                    "name": "Comment",
                    "alias": "Comment",
                    "type": "esriFieldTypeString",
                    "editable": true
               }, {
                    "name": "DateFound",
                    "alias": "Date Found",
                    "type": "esriFieldTypeDate",
                    "editable": true
                }],
                "relationships": [],
                "capabilities": "Editing"
            };

            //create feature layers based on the feature collections
            seepFeatureLayer = new FeatureLayer(seepFeatureCollection, {
                id: '1',
                editable: true
            });

            seepImagesFeatureLayer = new FeatureLayer(seepImagesFeatureCollection, {
                id: '2'
            });

            seepNameObjectLayer = new FeatureLayer(seepNameObjectCollection, {
                id: '3',
                visible: false
            });

            seepContactObjectLayer = new FeatureLayer(seepContactObjectCollection, {
                id: '4',
                visible: false
            });

            map.addLayers([seepFeatureLayer, seepImagesFeatureLayer]);

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