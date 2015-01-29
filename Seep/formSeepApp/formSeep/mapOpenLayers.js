define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./mapOpenLayers.html',
    "dojo/hash",
    "dojo/date",
    "dojo/date/stamp",
    "dojox/validate/web",
    "dojox/validate/us",
    "dojo/dom",
    "dojo/on"
    ], function (declare, lang, dojo, Dialog, _WidgetsInTemplateMixin, template, hash, date, stamp, validate, dom, on) {
    return declare('formSeep.templates.mapOpenLayers', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Video/Track Map',
        style: 'width:80%; height:80%',
        templateString: template,

        setID: function () {
        },
        
        cleanup: function() {
            this.hide();
        },
        
        startMap: function (uploadid) {
            mapOL = appConfig.baseUrl+"videotracks.html?UPLOADID="+uploadid;        	
        	this.mapOLNode.src = mapOL;
        },

       constructor: function (options) {
            lang.mixin(this, options);
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);
        },
    });
});