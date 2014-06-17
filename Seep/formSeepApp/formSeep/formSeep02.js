define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    'dijit/Dialog',
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./formSeep02.html',
    'dijit/form/Button',
    'dijit/form/Form',
    'dijit/form/TextBox',
    'dijit/form/ValidationTextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    'dijit/form/Textarea',
    "dojox/form/Uploader",
    "dijit/form/DateTextBox",
    "dojox/form/uploader/FileList",
    "dojo/hash",
    "dojo/date",
    "dojo/date/stamp",
    "dojox/validate/web",
    "dojox/validate/us",
    "dojo/dom",
    "dojo/on",
    "dojo/request",
    "dojo/json"
], function (declare, lang, dojo, Dialog, _WidgetsInTemplateMixin, template, hash, date, stamp, validate, dom, on, request, JSON) {
    return declare('formSeep.templates.formSeep02', [Dialog, _WidgetsInTemplateMixin], {

        title: 'Spring Image Form',
        style: 'width:600px',
        templateString: template,
        //        current: 'xx',
        signature: "xx",
        images: null,
        count: 0,
        springData: {
            latitude: null,
            logitude: null,
            date: ""
        },
        baseAddress: "http://overtexplorations.com/Seep_1.2/",
        keyGoogle: "AIzaSyAJeXarCpe7QTjf_XIrVbaAWnaoBDASGXA",
        timeZoneRequest: "https://maps.googleapis.com/maps/api/timezone/json",
        meta: function (images) {
            this.count = Object.keys(images).length - 1;
            this.images = images;
            this.imageFormFill();
            this.show();
        },
        setSpringData: function (data) {
            this.springData = data;
        },
        getUTC: function () {
            // open a modal dialog
            dialog = new Dialog({
                title: "Finding UTC Info",
                content: "Finding Time Zone ...",
                style: "width: 300px",
                closable: false
            });

            dialog.show();

            request.get(this.timeZoneRequest, {
                handleAs: "json",
                query: "?location=" + this.images[this.count].latitude + "," + this.images[this.count].longitude +
                    "&timestamp=" + this.images[this.count].time_taken +
                    "&sensor=false&key=" + this.keyGoogle
            }).then(function (data) {
                    this.DSToffsetNode.value = data.dstOffset;
                    this.UTCoffsetNode.value = data.rawOffset;
                    this.timeZoneID.value = data.timeZoneId;
                    this.timeZoneName.value = data.timeZoneName;

                    // close dialog
                    dialog.hide();
                },
                function (error) {
                    alert(error);

                    // close dialog
                    dialog.hide();
                });
        },
        imageFormFill: function () {
            if (this.count > 0) {
                this.count--;

                // portrait, landscape or square
                if (this.images[this.count].width > this.images[this.count].height) {
                    this.imageNode.width = 300;
                    this.imageNode.height = 200;
                } else if (this.images[this.count].width < this.images[this.count].height) {
                    this.imageNode.width = 200;
                    this.imageNode.height = 300;
                } else {
                    this.imageNode.width = 300;
                    this.imageNode.height = 300;
                }

                // link
                this.imageNode.src = this.baseAddress + this.images[this.count].file;

                // lat/long from spring or image
                if (this.images[this.count].latitude == "") {
                    this.latitudeNode.value = this.springData.latitude;
                    this.longitudeNode.value = this.springData.longitude;
                } else {
                    this.latitudeNode.value = this.images[this.count].latitude;
                    this.longitudeNode.value = this.images[this.count].longitude;
                }

                // date from spring or image
                if (this.images[this.count].date_taken == "") {
                    this.date_takenNode.value = this.springData.date;
                } else {
                    this.date_takenNode.value = this.images[this.count].date_taken;
                }

                // UTC has to be converted, has to bring-up a dialog because asynchronous request
                // http://www.earthtools.org/timezone/40.71417/-74.00639
                // https://maps.googleapis.com/maps/api/timezone/json?location=xx,yy&timestamp=tt&sensor=true&key=API_KEY
                this.time_takenNode.value = this.images[this.count].time_taken;
                this.getUTC();
            } else {
                this.hide();
            }
        },
        constructor: function (options) {
            lang.mixin(this, options);

            this.signature = options.sig;
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);

            this.seepSubmitImage.on("click", lang.hitch(this, function () {
                // get all the values
                var image = {
                    signature: this.signatureNode.value,
                    title: this.titleNode.value,
                    description: this.descriptionNode.value,
                    image: this.imageNode.src
                }

                // send to map
                // map.addImage(image);

                // continue
                this.imageFormFill();
            }));
        }
    });
});