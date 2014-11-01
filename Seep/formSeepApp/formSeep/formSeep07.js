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
    'dojo/text!./formSeep07.html',
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dijit/form/DateTextBox"
], function (declare, lang, dojo, dom, on, domStyle, domAttr, Dialog, _WidgetsInTemplateMixin, template) {
    return declare('formSeep.templates.formSeep07', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Track/Waypoints Annotation Form',
        style: 'width:auto',
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

        },
        
        
    });
});