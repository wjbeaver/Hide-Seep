define([
    'dojo/_base/declare',
    'dojo/_base/lang',
    'dojo',
    "dijit/_WidgetBase",
    "dijit/_TemplatedMixin", 
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./UTCcall.html',
    'dojox/layout/TableContainer',
    "dojo/hash",
    "dojox/validate/web",
    "dojox/validate/us",
    "dojo/dom",
    "dojo/on",
    "dojo/json"
], function (declare, lang, dojo, _WidgetBase, _TemplatedMixin, _WidgetsInTemplateMixin, template, TableContainer, hash, validate, dom, on, JSON) {
    return declare('UTCcall', [_WidgetBase, _TemplatedMixin, _WidgetsInTemplateMixin], {

        templateString: template,
        keyGoogle: "AIzaSyAJeXarCpe7QTjf_XIrVbaAWnaoBDASGXA",
        timeZoneRequest: "https://maps.googleapis.com/maps/api/timezone/json",
        UTC: [],
        taken: [],
         
        getUTCSuccess: function (data, widget) {
            
            widget.UTC[0] = data.timeZoneId;
            widget.time_zoneViewNode.innerHTML = widget.UTC[0];
	    
            var GPXTime = function (offsetHours, zoneDate, zoneTime) {
                    monthDays = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
                
                    function padZero (number) {
                            out = "";
                        
                            if (number<10) {
                                   out = "0" + number; 
                            } else {
                                    out = "" + number;
                            }
                        
                            return out;
                    };
        
                hour = parseInt(zoneTime[0])+offsetHours;
            
                day = parseInt(zoneDate[2]);
            
                month = parseInt(zoneDate[1]);
            
                monthIndex = month-1;
            
                year = parseInt(zoneDate[0]);
            
                if (year % 4>0) {
                    monthDays[1]+=1;
                }
    
                if (hour>23) {
                    hour=hour%24;
                    day = day + parseInt(hour/24);
                
                    if (day>monthDays[monthIndex]) {
                        day = 1;
                        month +=1;
                        if (month>12) {
                            month = 1;
                            year += 1;
                        }
                    }
                } else if (hour<0) {
                    hour = hour + 24;
                    day = day-1;
                
                    if (day<1) {
                    if (monthIndex>0) {
                       day= monthDays[monthIndex-1];
                    } else {
                        day=monthDays[11];
                        year -= 1;
                    };
                    };
                };
            
                return year + "-" + padZero(month) + "-" + padZero(day) + "T" + padZero(hour) + ":" + zoneTime[1] + ":" + zoneTime[2] + "Z";
            };
	        widget.UTC[1] = GPXTime(-1*((data.dstOffset + data.rawOffset)/3600), widget.taken.date.split("/"), widget.taken.time.split(":"));
            widget.time_UTCViewNode.innerHTML = widget.UTC[1];
            widget.UTCStatus.innerHTML = "";
       },
        
        getUTCError: function (error, widget) {
            alert(error);

            widget.UTCStatus.innerHTML = error;
        },
        
        getUTC: function (taken) {
            this.taken = taken;
            var query = "location=" + taken.latitude + "," + taken.longitude +
                    "&timestamp=" + taken.timestamp/1000 +
                    "&sensor=false&key=" + this.keyGoogle;
            
            this.UTCStatus.innerHTML = "Finding UTC...";        
            requestGoogle(this.timeZoneRequest, query, this.getUTCSuccess, this.getUTCError, this);
        },
        
        clearNode: function () {
            this.UTC = [];
            this.taken = [];
            
            this.UTCStatus.innerHTML = "";
            this.time_UTCViewNode.innerHTML = "";
            this.time_zoneViewNode.innerHTML = "";
        },
        
        constructor: function (options) {
            lang.mixin(this, options);

        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);
                            
        }
    });
});