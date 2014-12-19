define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/dom",
    "dojo/on",
    "dojo/dom-style",
    "dojo/dom-attr",
    "dojo/json",
    "dojox/xml/DomParser",
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep07.html',
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dijit/form/DateTextBox"
], function (declare, lang, dojo, dom, on, domStyle, domAttr, JSON, DomParser, Dialog, _WidgetsInTemplateMixin, template) {
    return declare('formSeep.templates.formSeep07', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Track',
        style: 'width:auto',
        templateString: template,
        tracks: null,
        count: 0,
        mode: "",

        constructor: function (options) {
            lang.mixin(this, options);

            this.signature = options.sig;
        },

        meta: function (tracks) {
            this.count = Object.keys(tracks).length-1;

            this.tracks = tracks;
            
            // create the tracks
            var indx = layers[5].objects.length;
            for (i=0;i<this.count;i++) {
                layers[5] = addTrackObject(layers[5]);
                var name = this.tracks[i].name;
                layers[5].objects[indx+i].attributes[0].value = name.split(".")[0];
    	    }
    	
            this.trackFormFill();
            this.show();
        },
        
        getIndexOfTrack: function (value) {
            indx = -1;
            
            var tracks = layers[5].objects;
            for (i=0;i<tracks.length;i++) {
                var attributes = tracks[i].attributes;
                if (attributes[0].value == value) {
                    indx = i;
                    break;
                }
            }
            
            return indx;
        },
        
        trackFromGPX: function (gpx_file) {
            var track = {
                points: [],
                time: [],
                elevation: []
            };
            var objChild;
            var trckChild;
            var trckSegChild;
            var track_obj;
            var points;
            var point;
            var gpx_obj = DomParser.parse(gpx_file);
            
            objChild = gpx_obj.childNodes;
            for (i=0;i<objChild.length;i++) {
                if (objChild[i].nodeName=="gpx") {
                    trckChild = objChild[i].childNodes;
                    for (j=0;j<trckChild.length;j++) {
                        if (trckChild[j].nodeName=="trk") {
                            trckSegChild = trckChild[j].childNodes;
                            for (k=0;k<trckSegChild.length;k++) {
                                if (trckSegChild[k].nodeName=="trkseg") {
                                    // only get one track and one segment
                                    track_obj = trckSegChild[k];
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            for (i = 0; i<track_obj.childNodes.length;i++) {
                if (track_obj.childNodes[i].nodeName=="trkpt") {
                
                    points = track_obj.childNodes[i];
                    track.points[track.points.length] = [points.attributes[0].nodeValue, points.attributes[1].nodeValue];
                    
                    for (k=0; k< points.childNodes.length;k++) {
                        if (points.childNodes[k].nodeName=="ele") {
                            track.elevation[track.elevation.length] = points.childNodes[k].childNodes[0].nodeValue;
                        } else if (points.childNodes[k].nodeName=="time") {
                            track.time[track.time.length] = points.childNodes[k].childNodes[0].nodeValue;
                        }
                    }
                }
            }
            
            return track;
        },
        
        trackFormFill: function () {
            if (this.count > 0) {
        	    this.mode = "add";
        	
                this.trackNumberNode.innerHTML= "#"+this.count;

                this.count--;
                
                var attributes = layers[0].objects[0].attributes;
                var coordinates = layers[0].objects[0].coordinates;
                
                var name = this.tracks[this.count].name;
                indx = this.getIndexOfTrack(name.split(".")[0]);
                var attributesTrack = layers[5].objects[indx].attributes;
                var coordinatesTrack = layers[5].objects[indx].coordinates;

                // UPLOADID
                attributesTrack[1].value = attributes[0].value;
                attributesTrack[1].node = this.UPLOADIDTrackNode;
                attributesTrack[1].node.set("value", attributesTrack[1].value);

                // TRACKID
                attributesTrack[0].node = this.TRACKIDTrackNode;
                attributesTrack[0].node.set("value", attributesTrack[0].value);

                // track
                var track = this.trackFromGPX(this.tracks[this.count].track);
                this.tracks[this.count].track = null;                
                attributesTrack[4].value = track;
                 
                 // type
                 attributesTrack[2].node = this.trackTypeNode;
                 
                 // title
                 attributesTrack[3].node = this.trackTitleNode;
                 
                 // description
                 attributesTrack[5].node = this.trackDescribeNode;
                
            } else {
                // going back to the main form, anything to do?
                this.hide();
            }
        },
        
        addTrack: function () {
                 var indx =  this.getIndexOfTrack(this.TRACKIDTrackNode.get("value"));
                 var  attributesTrack = layers[5].objects[indx].attributes;
                  
                  // type
                 attributesTrack[2].value = attributesTrack[2].node.get("value");
                 attributesTrack[2].node.set("value", 1);
                 
                 // title
                 attributesTrack[3].value = attributesTrack[3].node.get("value");
                 attributesTrack[3].node.set("value", "");
                 
                 // description
                 attributesTrack[5].value = attributesTrack[5].node.get("value");
                 attributesTrack[5].node.set("value", "");
       },
        
        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

           this.trackSubmitNode.on("click", lang.hitch(this, function () {
                // has to have a title
                if (this.trackTitleNode.get("value")!="") {
                    this.addTrack();
                    
                    // continue
                    this.trackFormFill();
                } else {
                    alert("Needs a Title!");
                }
            }));
        },
    });
});