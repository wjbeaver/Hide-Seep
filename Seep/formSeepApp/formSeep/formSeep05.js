define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dojo/dom",
    "dojo/on",
    "dojo/dom-style",
    "dojo/dom-attr",
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep05.html',
    "dijit/form/RadioButton",
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/layout/TabContainer",
    "dijit/form/TimeTextBox",
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dijit/form/DateTextBox"
], function (declare, lang, dojo, dom, on, domStyle, domAttr, Dialog, _WidgetsInTemplateMixin, template) {
    return declare('formSeep.templates.formSeep05', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Video Form',
        style: 'width: 700px;',
        templateString: template,

         setLongitude: function (value) {
            this.longitude = value;
        },

        setLatitude: function (value) {
            this.latitude = value;
        },
        
        constructor: function (options) {
            lang.mixin(this, options);

            this.signature = options.sig;
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

            this.coordType1Node.on("click", lang.hitch(this, function () {
            		    domStyle.set(dom.byId("radio2"), "display", "none");
            		    domStyle.set(dom.byId("radio3"), "display", "none");
            		    domStyle.set(dom.byId("radio1"), "display", "block");
            }));

            this.coordType2Node.on("click", lang.hitch(this, function () {
            		    domStyle.set(dom.byId("radio1"), "display", "none");
           		    domStyle.set(dom.byId("radio3"), "display", "none");
             		    domStyle.set(dom.byId("radio2"), "display", "block");
            }));

            this.coordType3Node.on("click", lang.hitch(this, function () {
            		    domStyle.set(dom.byId("radio1"), "display", "none");
            		    domStyle.set(dom.byId("radio2"), "display", "none");
            		    domStyle.set(dom.byId("radio3"), "display", "block");
            }));
            
            this.getVideoTrackNode.on("click", lang.hitch(this, function () {
             		    dialog_trackSubmit.show();
           }));
            
            this.submitVideosNode.on("click", lang.hitch(this, function () {
             		    
           }));
            
            this.addVideoNode.on("click", lang.hitch(this, function () {
             		    
           }));
            
            this.removeVideoNode.on("click", lang.hitch(this, function () {
             		    
           }));
            
            this.getVideoUTCNode.on("click", lang.hitch(this, function () {
             		    
           }));
        },
        
        
    });
});