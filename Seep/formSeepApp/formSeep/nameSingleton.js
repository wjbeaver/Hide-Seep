define([
    "esri/tasks/query",
    "esri/tasks/QueryTask",
    'dojo/_base/declare', 
    'dojo/_base/lang',
    'formSeepApp/formSeep/makeSingleton'], 
    function(Query, QueryTask, declare, lang, MakeSingleton)
{
    return MakeSingleton( declare('nameSingleton', [], {
        
        names: null,
        
         constructor: function (options) {
            lang.mixin(this, options);
            
            this.currentNames();
            
            // this.clearAllNames();
            // this.clearAllImages();
        },

        clearAllIds: function(featureIds, layerId) {
            var layer = map.getLayer(layerId);
            var attributes;
            var nameList = [];
            var namer;
            
            if (featureIds!=null) {
                for (i=0;i<featureIds.length;i++) {
                    attributes = {
                        OBJECTID: featureIds[i]
                    };
                    namer = {
                        attributes: attributes
                    };
            
                    nameList[nameList.length] = namer;                   
               }
           
               layer.applyEdits(null, null, nameList);
           }
        },
        
        clearAllNames: function() {
            var tableLayer = map.getLayer("7");
            
            var query = new Query();
            query.returnGeometry = false;
            
            query.where = "OBJECTID>0";
            
            tableLayer.queryIds(query, lang.hitch(this, function (featureIds) {
                this.clearAllIds(featureIds, "7");
            }));
            
            tableLayer = map.getLayer("8");
            
            query = new Query();
            query.returnGeometry = false;
            
            query.where = "OBJECTID>0";
            
            tableLayer.queryIds(query, lang.hitch(this, function (featureIds) {
                this.clearAllIds(featureIds, "8");
            }));
            
        },
        
        clearAllImages: function() {
            var tableLayer = map.getLayer("6");
            
            var query = new Query();
            query.returnGeometry = false;
            
            query.where = "OBJECTID>0";
            
            tableLayer.queryIds(query, lang.hitch(this, function (featureIds) {
                this.clearAllIds(featureIds, "6");
            }));
        },
        
        fetchCurrentNames: function(featureSet) {
            this.names = featureSet;
        },

        currentNames: function() {
                var layer = map.getLayer("7");
                var query = new Query();
                query.outFields = ["*"];
                query.returnGeometry = true;
                query.where = "NAMEID_PK!=''";
                query.num = 30;
    
                layer.queryFeatures(query, lang.hitch(this, function (featureSet) {
                    this.fetchCurrentNames(featureSet);
                }));                
        }
        
    }));  
});