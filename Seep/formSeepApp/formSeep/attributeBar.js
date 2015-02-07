define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/date/locale",
    'dojox/layout/FloatingPane',
    "dijit/_WidgetBase",
    "dijit/_TemplatedMixin", 
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./attributeBar.html',
    'dijit/form/Button',
    'dojox/layout/TableContainer',
    "dojo/hash",
    "dojo/date",
    "dojo/date/stamp",
    "dojo/dom",
    "dojo/on"
], function (declare, lang, dojo, locale, FloatingPane, _WidgetBase, _TemplatedMixin, _WidgetsInTemplateMixin, template, hash, date, stamp, dom, on) {
    return declare('attributeBar', [_WidgetBase, _TemplatedMixin, _WidgetsInTemplateMixin], {

        templateString: template,
        UPLOADID: "",
        TYPE: null,
        dateFound: null,
        device: null,
        flow: null,
        condition: null,
        accuracy: null,
        comment: null,
        describe: null,
        features: null,
        currentFeature: null,
        
        getTYPE: function (id) {
            var name = null;
            
            var types = this.currentFeature._graphicsLayer.types;
            for (i=0;i<types.length;i++) {
                if (types[i].id==id) {
                    name = types[i].name;
                    break;
                }
            }
            return name;
        },
        
        getCodedValue: function (name, code) {
            var label = null;
            var codedValue;
            
            if (code=="NA") {
                label = "NA";
            } else if (code!=null) {
                if (name=="Accuracy") {
                    var types = this.currentFeature._graphicsLayer.types;
                    for (i=0;i<types.length;i++) {
                        if (types[i].name=="Existing") {
                            codedValues = types[i].domains.Accuracy.codedValues;
                            for (j = 0; j<codedValues.length;j++) {
                                if (codedValues[j].code==code) {
                                    label = codedValues[j].name;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    var fields = this.currentFeature._graphicsLayer.fields;
                
                    for (i=0;i<fields.length;i++) {
                        if (fields[i].name == name) {
                            codedValues = fields[i].domain.codedValues;
                            for (j=0;j<codedValues.length;j++) {
                                if (codedValues[j].code==code) {
                                    label = codedValues[j].name;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            return label;
        },
        
        resetAttributes: function () {
     		attributes = appConfig.layers[0].objects[0].attributes;

            this.TYPE = this.getTYPE(attributes[4].value);
            this.TYPENode.innerHTML = this.TYPE;
            
            if (attributes[1].value==null) {
                this.dateFound = attributes[1].value;
                this.dateFoundNode.innerHTML = "null";
            } else {
                this.dateFound = attributes[1].value;
                this.dateFoundNode.innerHTML = locale.format(this.dateFound, {selector:"date"});
            }

            this.device = this.getCodedValue("Device", attributes[2].value);
            this.deviceNode.innerHTML = this.device;

            this.flow = this.getCodedValue("Flow", attributes[3].value);
            this.flowNode.innerHTML = this.flow;

            this.condition = this.getCodedValue("Condition", attributes[5].value);
            this.conditionNode.innerHTML = this.condition;
          
            this.accuracy = this.getCodedValue("Accuracy", attributes[6].value);
            this.accuracyNode.innerHTML = this.accuracy;
            
            this.comment = attributes[7].value;
            this.commentNode.innerHTML = this.comment;
            
            this.describe = attributes[8].value;
            this.describeNode.innerHTML = this.describe;
            
            // find the names
            this.namesNode.innerHTML = appConfig.dialog_seepMain.namesList();
        },
        
        setAttributes: function () {
            appConfig.dialog_seepMain.clearLayers();
            appConfig.layers[0] = addSeepObject(appConfig.layers[0]);
     		attributes = appConfig.layers[0].objects[0].attributes;

            // get attributes
            this.UPLOADID = this.currentFeature.attributes.UPLOADID_PK;
            attributes[0].value = this.UPLOADID;
            
            this.TYPE = this.getTYPE(this.currentFeature.attributes.TYPE);
            this.TYPENode.innerHTML = this.TYPE;
            attributes[4].value = this.currentFeature.attributes.TYPE;            
            
            if (this.currentFeature.attributes.DateFound==null) {
                this.dateFound = this.currentFeature.attributes.DateFound;
                this.dateFoundNode.innerHTML = "null";
            } else {
                this.dateFound = new Date(this.currentFeature.attributes.DateFound);
                this.dateFoundNode.innerHTML = locale.format(this.dateFound, {selector:"date"});
            }
            attributes[1].value = this.dateFound;        

            this.device = this.getCodedValue("Device", this.currentFeature.attributes.Device);
            this.deviceNode.innerHTML = this.device;
            attributes[2].value = this.currentFeature.attributes.Device;        

            this.flow = this.getCodedValue("Flow", this.currentFeature.attributes.Flow);
            this.flowNode.innerHTML = this.flow;
            attributes[3].value = this.currentFeature.attributes.Flow;        

            this.condition = this.getCodedValue("Condition", this.currentFeature.attributes.Condition);
            this.conditionNode.innerHTML = this.condition;
            attributes[5].value = this.currentFeature.attributes.Condition;        
          
            this.accuracy = this.getCodedValue("Accuracy", this.currentFeature.attributes.Accuracy);
            this.accuracyNode.innerHTML = this.accuracy;
            attributes[6].value = this.currentFeature.attributes.Accuracy;        
            
            this.comment = this.currentFeature.attributes.Comment;
            this.commentNode.innerHTML = this.comment;
            attributes[7].value = this.comment;        
            
            this.describe = this.currentFeature.attributes.Describe;
            this.describeNode.innerHTML = this.describe;
            attributes[8].value = this.describe;        
            
            // find the names
            appConfig.dialog_seepMain.setEdit(this);            
        },

       setID: function (features) {
            this.features = features;
            
            this.currentFeature = features[0];
            
            // turn on multiple
//            if (features>1) {
//            }
            
            this.setAttributes();
        },
        
       constructor: function (options) {
            lang.mixin(this, options);
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);
            
            dojo.destroy(this.attributeBarNode.closeNode);

            this.attributeCloseNode.on("click", lang.hitch(this, function () {
                this.attributeBarNode.hide();
                appConfig.map.barShow = false;     
            }));

            this.editNode.on("click", lang.hitch(this, function () {
                appConfig.dialog_seepMain.show();         
            }));

            this.deleteNode.on("click", lang.hitch(this, function () {
                if (confirm("Deleting will remove all photos, videos and tracks associated with this spring. OK?")) {
                    appConfig.dialog_seepMain.deleteSeepFeatures();
                    appConfig.springDeleted();
                }
            }));

            this.zoomImageNode.on("click", lang.hitch(this, function () {
                appConfig.map.setZoom(15);
            }));

            this.viewImageNode.on("click", lang.hitch(this, function () {
                appConfig.dialog_image.imageSetEdit(this.UPLOADID);
            }));

            this.viewVTNode.on("click", lang.hitch(this, function () {
                appConfig.map_open_layers.show();          
                appConfig.map_open_layers.startMap(this.UPLOADID);
            }));
            
            this.viewFirstNode.on("click", lang.hitch(this, function () {
            
            }));

            this.viewPreviousNode.on("click", lang.hitch(this, function () {
            
            }));

            this.viewNextNode.on("click", lang.hitch(this, function () {
            
            }));

            this.viewLastNode.on("click", lang.hitch(this, function () {
            
            }));
        }
    });
});