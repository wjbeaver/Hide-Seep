define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep08.html',
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    'dijit/form/Textarea',
    "dojo/request/xhr",
    "dojo/dom",
    "dojo/on",
    "dojo/domReady!"
], function (declare, lang, dojo, Dialog, _WidgetsInTemplateMixin, template, xhr, dom, on) {
    return declare('contact.templates.contact', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Admin Log In',
        style: 'width:580px',
        templateString: template,

        constructor: function (options) {
            lang.mixin(this, options);

         },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

        this.adminSubmit.on("click", lang.hitch(this, function (e) {
            // Get the result node
            var resultNode = this.loginResultNode;
            resultNode.innerHTML="Sending...";
        
            dojo.stopEvent(e);
        
            // Post the form information
            dojo.xhrPost({
                    form: this.adminForm.domNode
                
            }).then(function (result) {
                resultNode.innerHTML = result;
            }, function (err) {
                resultNode.innerHTML = "Contact Form Failed!";
            });                        
            }));
        }
   });
});

                        