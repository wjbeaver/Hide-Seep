var dialog_image_loader;
var dialog_image;
var dialog;
var dialog_imagePan;
var dialog_seepMain;
var dialog_video;
var dialog_trackSubmit;
var dialog_track;
var requestGoogle;
var currentWidget;
var admin = false;

var layers;

dojo.declare("layer.seep",null,{
  	name: "",
	type: "",
	objects: [
	],
	constructor: function(args) {
		dojo.safeMixin(this,args);
	}
});

dojo.declare("layer.object",null,{
	coordinates: {
		latitude: 0,
		latNode: "",
		longitude: 0,
		longNode: ""
	},
	attributes: [
	],
	constructor: function(args) {
		dojo.safeMixin(this,args);
	}
});

dojo.declare("layer.attribute",null,{
	name: "",
	label: "",
	value: "",
	valueLabel: "",
	node: "",
	constructor: function(args) {
		dojo.safeMixin(this,args);
	}
});

dojo.declare("layer.attributesSeep", null, {
		EstimatedFlow: null,
		UPLOADID_PK: null,
		DateFound: null,
		Device: null,
		SPRINGID_FK: null,
		Flow: null,
		TYPE: 0,
		Condition: null,
		Accuracy: "NA",
		Comment: null,
		Describe: null		
});

dojo.declare("layer.attributesImage", null, {
});

var seepLayer = new layer.seep({
	name: "seepLayer",
	type: "point",
	objects: []
});

var nameLayer = new layer.seep({
	name: "nameLayer",
	type: "table",
	objects: []
});

var nameFeatureLayer = new layer.seep({
	name: "nameFeatureLayer",
	type: "table",
	objects: []
});

var imageLayer = new layer.seep({
	name: "imageLayer",
	type: "point",
	objects: []
});

var photographerLayer = new layer.seep({
	name: "photographerLayer",
	type: "table",
	objects: []
});

var videoLayer = new layer.seep({
	name: "videoLayer",
	type: "point",
	objects: []
});

var videographerLayer = new layer.seep({
	name: "videographerLayer",
	type: "table",
	objects: []
});

var trackPointLayer = new layer.seep({
	name: "trackPointLayer",
	type: "point",
	objects: []
});

var trackLineLayer = new layer.seep({
	name: "trackLineLayer",
	type: "line",
	objects: []
});

var trackTableLayer = new layer.seep({
	name: "trackTableLayer",
	type: "table",
	objects: []
});

layers = [seepLayer, nameLayer, nameFeatureLayer, imageLayer, photographerLayer, videoLayer, videographerLayer, trackPointLayer, trackLineLayer, trackTableLayer];


var addNameObject = function(nameLayer) {
	nameLayer.objects[nameLayer.objects.length] = new layer.object({
	attributes: [
		new layer.attribute({
			name: "NAMEID_PK",
			label: "NAMEID"
		}),
		new layer.attribute({
			name: "honorific",
			label: "Honorific"
		}),
		new layer.attribute({
			name: "first_name",
			label: "First Name"
		}),
		new layer.attribute({
			name: "middle_name",
			label: "Middle Name"
		}),
		new layer.attribute({
			name: "last_name",
			label: "Last Name"
		}),
		new layer.attribute({
			name: "email",
			label: "Email"
		})
	]});
	
	return nameLayer;
}

var addNameFeatureObject = function(nameFeatureLayer) {
	nameFeatureLayer.objects[nameFeatureLayer.objects.length] = new layer.object({
	attributes: [
		new layer.attribute({
			name: "NAMEID_FK",
			label: "NAMEID"
		}),
		new layer.attribute({
			name: "FEATUREID_FK",
			label: "FEATUREID"
		}),
		new layer.attribute({
			name: "FEATUREID_TYPE",
			label: "Feature Type"
		})
	]});
	
	return nameFeatureLayer;
}

/*
	TYPE
	0 - Seep
	1 - Image
	2 - Video
	3 - Track
*/

var addImageObject = function(imageLayer) {
	imageLayer.objects[imageLayer.objects.length] = new layer.object({
	attributes: [
		new layer.attribute({
			name: "IMAGEID_PK",
			label: "IMAGEID"
		}),
		new layer.attribute({
			name: "UPLOADID_FK",
			label: "UPLOADID"
		}),
		new layer.attribute({
			name: "title",
			label: "Title"
		}),
		new layer.attribute({
			name: "description",
			label: "Description"
		}),
		new layer.attribute({
			name: "dateTaken",
			label: "Date Taken"
		}),
		new layer.attribute({
			name: "timeTaken",
			label: "Time Taken"
		}),
		new layer.attribute({
			name: "timeZone",
			label: "Time Zone"
		}),
		new layer.attribute({
			name: "UTC",
			label: "UTC"
		}),
		new layer.attribute({
			name: "elevation",
			label: "Elevation"
		}),
		new layer.attribute({
			name: "bearing",
			label: "Bearing"
		})
	]});
	
	return imageLayer;
}

var addSeepObject = function(seepLayer) {
	seepLayer.objects[seepLayer.objects.length] = new layer.object({
	attributes: [
		new layer.attribute({
			name: "UPLOADID_PK",
			label: "UPLOADID"
		}),
		new layer.attribute({
			name: "DateFound",
			label: "Date Found"
		}),
		new layer.attribute({
			name: "Device",
			label: "Device"
		}),
		new layer.attribute({
			name: "Flow",
			label: "Flow"
		}),
		new layer.attribute({
			name: "TYPE",
			label: "Spring Type"
		}),
		new layer.attribute({
			name: "Condition",
			label: "Condition"
		}),
		new layer.attribute({
			name: "Accuracy",
			label: "Accuracy"
		}),
		new layer.attribute({
			name: "Comment",
			label: "Comment"
		}),
		new layer.attribute({
			name: "Describe",
			label: "Describe"
		})
	]});
	
	return seepLayer;
};

define([
'require',
'dojo/ready',
'dijit/registry',
'dojo/dom',
"dojo/dom-style",
"dojo/request",
'dijit/form/Button',
'dijit/form/Form',
"dojo/domReady!"
], function (require, ready, registry, dom, domStyle, request) {

    var app = {};

    ready(function () {
        app.seepForm({
            onSuccess: function (response) {
                console.log(response.message);
                this.onExecute(); // Hide dialog.
                app.alert('Success!');
            },
            onFailure: function (response) {
                console.log(response.message);

                if (response.status == "Validation Error") {
                    app.alert(response.message);
                } else {
                    app.alert('Failure!');
                }
            },
            UPLOADID: "",
            longitude: 0,
            latitude: 0
        });

        require(["dojo/on"], function (on) {
            on(dom.byId('btnStart'), 'click', app.onButtonClick);
            on(dom.byId('adminSubmit'), 'click', app.onSubmitClick);
            on(dom.byId("logoutButton"), 'click', app.logoutClick);
        });
        
        requestGoogle = function (url, query, successFunction, errorFunction, widget) {
        	currentWidget= widget;
        	
            request(url, {
                handleAs: "json",
                method: "GET",
                headers:{'X-Requested-With': null},
		query: query
            }).then(function (data) {
            	    successFunction(data, currentWidget);
                },
                function (error) {
                	errorFunction(error, currentWidget);
                });
        };

        console.log('loaded!');
    });

    /**
     * Create seepForm dialog.
     */
    app.seepForm = function (options) {
        require(['formSeepApp/formSeep/formSeep', 'formSeepApp/formSeep/formSeep01', 'formSeepApp/formSeep/formSeep02', 'formSeepApp/formSeep/formSeep03', 'formSeepApp/formSeep/formSeep04', 'formSeepApp/formSeep/formSeep05', 'formSeepApp/formSeep/formSeep06', 'formSeepApp/formSeep/formSeep07'], function (Dialog, Dialog_image_loader, Dialog_image, Dialog_imagePan, Dialog_seepMain, Dialog_video, Dialog_trackSubmit, Dialog_track) {
            if (typeof dialog === "undefined") {
               options.closable = true;
               dialog = new Dialog(options);
            }

            if (typeof dialog_seepMain === "undefined") {
               options.closable = true;
                dialog_seepMain = new Dialog_seepMain(options);
            }

            if (typeof dialog_image_loader === "undefined") {
                options.closable = true;
                dialog_image_loader = new Dialog_image_loader(options);
            }

            if (typeof dialog_image === "undefined") {
                options.closable = false;
                dialog_image = new Dialog_image(options);
                dialog_image_loader.setDialog_image(dialog_image);
            }
             if (typeof dialog_imagePan === "undefined") {
               options.closable = true;
                dialog_imagePan = new Dialog_imagePan(options);
            }
                       
            if (typeof dialog_video === "undefined") {
               options.closable = true;
                dialog_video = new Dialog_video(options);
            }

            if (typeof dialog_trackSubmit === "undefined") {
               options.closable = true;
                dialog_trackSubmit = new Dialog_trackSubmit(options);
            }

            if (typeof dialog_track === "undefined") {
               options.closable = false;
                dialog_track = new Dialog_track(options);
            } 
        });
    };

    /**
     * Display alert dialog.
     */
    app.alert = function (message) {
        require(['dijit/Dialog'], function (Dialog) {
            var dialog = new Dialog({
                title: 'Alert',
                style: 'width:400px',
                content: message
            });

            return dialog;
        });
    };
    /**
     * Button click action.
     */
    app.onButtonClick = function () {
    	layers[0] = addSeepObject(layers[0]);
    	
    	indx = layers[0].objects.length-1;
    	
    	layers[0].objects[indx].attributes[0].value = generateUUID();
    	
    	dialog.setID();
        dialog.show();
    };
    
    app.onSubmitClick = function () {
		var resultNode = dom.byId('loginResult');
		resultNode.innerHTML="Sending...";
		
		var loginForm = dom.byId('loginForm');
		var password = dom.byId('adminPassword');
		password.value = CryptoJS.MD5(password.value);
		
		// Post the form information
		dojo.xhrPost({
				form: loginForm
				
		}).then(function (result) {
		    if (result=="pass") {
		        admin = true;

			    resultNode.innerHTML = "Welcome Admin!";
			    
			    var login = dom.byId("login");
			    var logout = dom.byId("logout");
			    
			    domStyle.set(login, "display", "none");
			    domStyle.set(logout, "display", "block");

		        // do something to the map 
		    } else {
			    resultNode.innerHTML = "Wrong Username and Password";
		    }
		    
		}, function (err) {
			resultNode.innerHTML = "Contact Form Failed!";
		});        
    };
    
    app.logoutClick = function () {
		        admin = false;
			    
			    var login = dom.byId("login");
			    var logout = dom.byId("logout");
			    
			    domStyle.set(login, "display", "block");
			    domStyle.set(logout, "display", "none");

		        // do something to the map 
    };
    return app; 
});