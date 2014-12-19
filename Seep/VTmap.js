var map, layer;
var layerURL = "http://services.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer";

function init() {
    var jsonp = new OpenLayers.Protocol.Script();

    jsonp.createRequest(layerURL, {
        f : 'json',
        pretty : 'true'
    }, initMap);
}

function initMap(layerInfo){

     /***********************************************************************************
      *ArcGIS Online Basemap (Topographic)
      ***********************************************************************************/
    var baseLayer = new OpenLayers.Layer.ArcGISCache("Topo", layerURL, {
        layerInfo : layerInfo,
        attribution: "Springs Stewardship Institute | Esri, DeLorme, FAO, USGS, NOAA, EPA"
    });

    var video = new OpenLayers.Layer.Vector("Video", {
        strategies: [new OpenLayers.Strategy.Fixed()],
        protocol: new OpenLayers.Protocol.WFS({
            url: "http://wbeaver2.xen.prgmr.com:8080/geoserver/wfs",
            featurePrefix: 'geo1',
            featureType: "video",
            featureNS: "http://wbeaver2.xen.prgmr.com:8080/geoserver/geo1",
            geometryName: "geom"
        })
    });

    /*
     * Make sure our baselayer and our map are synced up.
     */
    map = new OpenLayers.Map('mapOL', {
        maxExtent : baseLayer.maxExtent,
        units : baseLayer.units,
        resolutions : baseLayer.resolutions,
        numZoomLevels : baseLayer.numZoomLevels,
        tileSize : baseLayer.tileSize,
        projection: new OpenLayers.Projection("EPSG:3857"),
        displayProjection: new OpenLayers.Projection("EPSG:3857"),
        units: "m"
    });
    
    map.addLayers([baseLayer, video]);
    
    map.zoomToExtent(new OpenLayers.Bounds(-10238892.81285323,2744395.0635501994,-5028944.9649370015,5870363.772299937));
};

init();
