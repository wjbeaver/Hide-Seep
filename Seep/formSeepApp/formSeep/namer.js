define([
    "esri/tasks/query",
    "esri/tasks/QueryTask",
    'dojo/_base/declare',
    "dojo/parser",
    "dojo/ready",
    'dojo/_base/lang',
    'dojo',
    "dojo/dom-style",
    'formSeepApp/formSeep/nameSingleton',
    "dijit/_WidgetBase",
    "dijit/_TemplatedMixin", 
    'dijit/_WidgetsInTemplateMixin',
    'dojo/text!./namer.html',
    'dijit/form/Button',
    'dijit/form/TextBox',
    'dojox/layout/TableContainer',
    "dijit/form/Select",
    "dojo/dom",
    "dojo/on",
    "dojo/json"
], function (Query, QueryTask, declare, parser, ready, lang, dojo, domStyle, NameSingleton,
            _WidgetBase, _TemplatedMixin, _WidgetsInTemplateMixin, template, TableContainer, dom, on, JSON) {
    declare('namer', [_WidgetBase, _TemplatedMixin, _WidgetsInTemplateMixin], {

        templateString: template,
        mode: "add",
        unique: false,

         _setUniqueAttr: function(/*Boolean*/ unique){
             this._set("unique", unique);
         },
         
        nameType: "",
        TYPE: {"Seep":0, "Image":1, "Video":2},
        
         _setNameTypeAttr: function(/*String*/ nameType){
             this._set("nameType", nameType);
         },
         
         newNameListTitle: "",
         
         _setNewNameListTitle: function(/*String*/ newNameListTitle) {
            this._set("newNameListTitle", newNameListTitle);         
         },
         
         featureValue: "",
        
         _setFeatureValue: function(/*String*/ featureValue){
             this._set("featureValue", featureValue);
         },
         
         width: "100%",

        _setWidth: function(/*String*/ width){
             this._set("width", width);
        },
        
        nameSingleton: null,
        names: [],
        featureNames: [],
        attributeBar: null,
        editRemove: [],
        
         constructor: function (options) {
            lang.mixin(this, options);
            
            // get the names list
            this.nameSingleton = NameSingleton.getInstance();
        },

       namesList: function () {
            var list = "<p>";
            var name;
            
            for (i=0;i<this.names.length;i++) {
                name = this.names[i];
                
                list += this.createName(this.getHonorific(name.Honorific), name.FirstName, name.MiddleName, name.LastName)+"<br/>";
            }
            
            return list+"</p>";
        },
        
        createOptionFromName: function (indx) {
            var options = [];
            
            if (indx==0) {
                options[0] = {
                    label: this.newNameListTitle,
                    value: 0
                };
            }
            
            var name = this.names[indx];
            
            options[options.length] = {
                label: this.createName(this.getHonorific(name.Honorific), name.FirstName, name.MiddleName, name.LastName),
                value: name.NAMEID_PK
            };                
            
            this.newNameListNode.addOption(options);
        },
        
        createFromName: function (indx) {
            var name = this.names[indx];
            
            appConfig.layers[1] = addNameObject(appConfig.layers[1]);
                
            nameAttributes = appConfig.layers[1].objects[appConfig.layers[1].objects.length-1].attributes;
                
            nameAttributes[0].value = name.NAMEID_PK;
            
            nameAttributes[1].value = name.Honorific;
            nameAttributes[1].node = this.honorificNode;
            
            nameAttributes[2].value = name.FirstName;
            nameAttributes[2].node = this.firstNameNode;
            
            nameAttributes[3].value = name.MiddleName;
            nameAttributes[3].node = this.middleNameNode;
            
            nameAttributes[4].value = name.LastName;
            nameAttributes[4].node = this.lastNameNode;
            
            nameAttributes[5].value = name.Email;                
            nameAttributes[5].node = this.emailNode;                
        },
        
        selectFromList: function (indx) {
            var name = this.names[indx];
            
        	this.honorificNode.set("value", name.Honorific);
        	this.firstNameNode.set("value", name.FirstName);
        	this.middleNameNode.set("value", name.MiddleName);
        	this.lastNameNode.set("value", name.LastName);
        	this.emailNode.set("value", name.Email);
        },
        
        fetchNames: function (featureSet, attributeBar, end) {
            if (featureSet.features.length>0) { 
                var indx = this.names.length;        
                this.names[this.names.length] = featureSet.features[0].attributes;
                
                // create list
                this.createFromName(indx);
                
                // create option
                this.createOptionFromName(indx);
                
                if (indx==0) {
                    this.selectFromList(indx);
                } 
                
                if (end) {
                    this.newNameListNode.set("value", this.names[0].NAMEID_PK);
                    if (this.nameType=="Seep" || this.nameType=="") {
                        attributeBar.namesNode.innerHTML = this.namesList();
                    }
                }             
            } else {
                alert("No names, problem!");
            }
        },
        
        fetchNameIds: function (featureSet) {
            if (featureSet.features.length>0) {
                var layer = appConfig.map.getLayer("7");
                var query = new Query();
                query.outFields = ["*"];
                query.returnGeometry = false;
                
                var attributes;
                var indx = featureSet.features.length-1;
                var end = false;
                
                for (i=0;i<featureSet.features.length;i++) {
                    attributes = featureSet.features[i].attributes;
                    appConfig.layers[2] = addNameFeatureObject(appConfig.layers[2]);
                    
                    featureNameAttributes = appConfig.layers[2].objects[appConfig.layers[2].objects.length-1].attributes;
                    
                    featureNameAttributes[0].value = attributes.NAMEID_FK;
                    featureNameAttributes[1].value = attributes.UPLOADID_FK;
                    featureNameAttributes[2].value = attributes.TYPE;
                    
                    this.featureNames[this.featureNames.length] = attributes;                  
                    
                    query.where = "NAMEID_PK='"+attributes.NAMEID_FK+"'";
                    
                    if (i==indx) {
                        end = true;
                    }
                    
                    layer.queryFeatures(query, lang.hitch(this, function (featureSet) {
                        this.fetchNames(featureSet, this.attributeBar, end);
                    }));                
                }
                
            } else {
                alert("No name ids, problem!");
            }
        },
        
        addNames: function() {
            var names = appConfig.layers[1].objects;
            var attributes;
            var nameAttributes;
            var nameList = [];
            for (i=0;i<names.length;i++) {
                attributes = names[i].attributes;
                
                nameAttributes = new layer.attributesName();
                
                nameAttributes.NAMEID_PK = attributes[0].value;
                nameAttributes.Honorific = attributes[1].value;
                nameAttributes.FirstName = attributes[2].value;
                nameAttributes.MiddleName = attributes[3].value;
                nameAttributes.LastName = attributes[4].value;
                nameAttributes.Email = attributes[5].value;
                
                var name = {
                    attributes: nameAttributes
                };
                
                nameList[nameList.length] = name;
            }
            tableLayer = appConfig.map.getLayer("7");
                     
            tableLayer.applyEdits(nameList, null, null);
            
            var featureNames = appConfig.layers[2].objects;
            var featureNameList = [];
            for (i=0;i<featureNames.length;i++) {
                attributes = featureNames[i].attributes;
                
                nameAttributes = new layer.attributesFeatureName();
                
                nameAttributes.NAMEID_FK = attributes[0].value;
                nameAttributes.UPLOADID_FK = attributes[1].value;
                nameAttributes.TYPE = attributes[2].value;

                var name = {
                    attributes: nameAttributes
                };
                
                featureNameList[featureNameList.length] = name;
            }
                
            var tableLayer = appConfig.map.getLayer("8");

            tableLayer.applyEdits(featureNameList, null, null, lang.hitch(this, function (featureEditResults) {
                this.nameSingleton.currentNames();
            }));
            
        },
        
        updateNames: function(id) {
            var nameList = [];
            var tableLayer = appConfig.map.getLayer("7");
            var namer;
                     
            for (i=0;i<this.names.length;i++) {
                var name = this.names[i];
                var attributes = this.getAttributes(name.NAMEID_PK); 
                
                if (attributes!=null) {
                    name.Honorific = attributes[1].value;              
                    name.FirstName = attributes[2].value;              
                    name.MiddleName = attributes[3].value;              
                    name.LastName = attributes[4].value;              
                    name.Email = attributes[5].value;
                
                    namer = {
                        attributes: name
                    };
                
                    nameList[nameList.length] = namer;
                }                   
            }
            
            tableLayer.applyEdits(null, nameList, null, lang.hitch(this, function (featureEditResults) {
                this.nameSingleton.currentNames();
            }));

            // delete any names removed
            this.deleteNames(id);
        },
        
        doDeleteName: function (featureIds, nameId) {
            if (featureIds.length==0) {
                var tableLayer = appConfig.map.getLayer("7");
                for (i=0;i<this.names.length;i++) {
                    if (this.names[i].NAMEID_PK==nameId) {
                        namer = {
                            attributes: this.names[i]
                        }
                    }
                }
                         
                tableLayer.applyEdits(null, null, nameList, lang.hitch(this, function (featureEditResults) {
                    this.nameSingleton.currentNames();
                }));
            }
        },
        
        deleteNames: function(id) {
            if (id!=null) {
                var nameList = [];
                var tableLayer = appConfig.map.getLayer("8");
                var name;
                for (i=0;i<this.editRemove.length;i++) {
                    for (j=0;j<this.featureNames.length;j++) {
                        if (this.editRemove[i]==this.featureNames[j].NAMEID_FK && id==this.featureNames[j].UPLOADID_FK) {
                            name = this.featureNames[j];
                 
                            namer = {
                                attributes: name
                            };
                
                            nameList[nameList.length] = namer;                     
                       }
                    }    
                }
            
                tableLayer.applyEdits(null, null, nameList);
            
                // now query the featureNames for any other links to that name, if not, remove the name
                for (i=0;i<this.editRemove.length;i++) {
                    var query = new Query();
                    query.returnGeometry = false;
                    query.where = "UPLOADID_FK='"+id+"'";

                    tableLayer.queryIds(query, lang.hitch(this, function (featureIds) {
                        this.doDeleteName(featureIds, this.editRemove[i]);
                    }));
                }
            }              
        },
        
        deleteAllNames: function(id) {
            var tableLayer = appConfig.map.getLayer("8");
            
            var query = new Query();
            query.returnGeometry = false;
            query.outFields = ["*"];
            query.where = "UPLOADID_FK='"+id+"'";

            tableLayer.queryIds(query, lang.hitch(this, function (featureIds) {
                this.nameSingleton.clearAllIds(featureIds, "8");
            }));
        },
        
        editNames: function(uploadID, featureId, featureType, attributeBar) {
            this.setEdit();
            
            this.clearNode();
            
            this.attributeBar = attributeBar;
            
            var layer = appConfig.map.getLayer("8");
            var query = new Query();
            query.outFields = ["*"];
            query.returnGeometry = false;
            var type = 0;
            
            switch (featureType) {
                case "Image":
                    this.newNameListTitle = "Photographer";
                    type = 1;
                    break;
                case "Springs":
                    this.newNameListTitle = "Discoverer(s)";
                    type = 0;
                    break;
                case "Video":
                    this.newNameListTitle = "Videographer";
                    type = 2;
            }
            
            // more must be done, this is only for type = 0
            query.where = "UPLOADID_FK='"+featureId+ "'";
            
            layer.queryFeatures(query, lang.hitch(this, function (featureSet) {
                this.fetchNameIds(featureSet);
            }));
        },
        
        setEdit: function () {
            this.mode = "edit";
            domStyle.set(this.nameListNode.domNode, "display", "none");
            domStyle.set(this.addNameNode.domNode, "display", "none");
            domStyle.set(this.updateNameNode.domNode, "display", "block");
         },

        setAdd: function () {
            this.mode = "add";
            domStyle.set(this.nameListNode.domNode, "display", "block");
            domStyle.set(this.addNameNode.domNode, "display", "block");
            domStyle.set(this.updateNameNode.domNode, "display", "none");
         },

        createName: function (honorific, fName, mName, lName) {
        	
        	if (fName != "") {
        		honorific += " " + fName;
        	}
        	
        	if (mName != "") {
        		honorific += " " + mName;
        	}
        	
        	return honorific + " " + lName;
        },
        
        getHonorific: function (honorific) {
        	if (honorific!="Miss") {
        		honorific+=".";
        	}
        	return honorific
        },
        
        clearName: function () {
        	this.honorificNode.set("value", "Mr");
        	this.firstNameNode.set("value", "");
        	this.middleNameNode.set("value", "");
        	this.lastNameNode.set("value", "");
        	this.emailNode.set("value", "");
        },
        
        clearNode: function() {
            this.clearName();
            
            var indx = this.nameListNode.options.length-1;
            
            for (i=indx;i>-1;i--) {
        		this.nameListNode.removeOption(this.nameListNode.options[i]);
        	}      	
/*
            this.nameListNode.addOption({
                label: "Names",
                value: 0
            });
*/          
            this.nameListNode.set("value", 0);

            indx = this.newNameListNode.options.length-1;
            
            for (i=indx;i>-1;i--) {
        		this.newNameListNode.removeOption(this.newNameListNode.options[i]);
        	}
/*
            this.newNameListNode.addOption({
                label: this.newNameListTitle,
                value: 0
            });
            
            this.newNameListNode.set("value", 0);
*/            
            this.names = [];
        },
        
        clearNames: function()  {
            // remove names from layer
            var i;
            for (i=1;i<this.newNameListNode.options.length;i++) {
                var value = this.newNameListNode.options[i].value;
                
                var indx = this.getIndexOfFeatureNameFromValue(value);
                 // remove name feature layer object
                 appConfig.layers[2].objects.splice(indx,1);
           
                 // remove name from layer if this is the only nameFeature
                 if (indx==-1) {
                     // remove from name layer object
                     appConfig.layers[1].objects.splice(this.getIndexOfNameFromValue(value),1);
                 }    
           }
            
            this.clearNode();
        },
        
        findNameCreateOption: function(value) {
            var names = appConfig.layers[1].objects;
            for (i=0;i<names.length;i++) {
                var attributes = names[i].attributes;
                
                if (attributes[0].value==value) {
                    return {
                        label: this.createName(this.getHonorific(attributes[1].value), attributes[2].value, attributes[3].value, attributes[4].value),
                        value: attributes[0].value
                    };
                }
            }
            
            return null;
        },
        
        updateNameList: function() {
            var options = [];
            
            var names = appConfig.layers[1].objects;
            
            for (i=0;i<names.length;i++) {
                var attributes = names[i].attributes;
                options[options.length] = {
                    label: this.createName(this.getHonorific(attributes[1].value), attributes[2].value, attributes[3].value, attributes[4].value),
                    value: attributes[0].value
                };
            }
            
        	// now add the old names
            var oldNames = this.nameSingleton.names.features;
                    	
            for (i=0;i<oldNames.length;i++) {
                var attributes = oldNames[i].attributes;
                options[options.length] = {
                    label: this.createName(this.getHonorific(attributes.Honorific), attributes.FirstName, attributes.MiddleName, attributes.LastName),
                    value: attributes.NAMEID_PK
                };
            }
            
            if (options.length>0) {
                this.nameListNode.addOption(options);
                this.nameListNode.set("value", options[0].value);
        	}
        },
        
        fillNameList: function() {
            if (this.newNameListNode.options.length==0) {
                this.newNameListNode.addOption({
                    label: this.newNameListTitle,
                    value: 0
                });
            
                this.newNameListNode.set("value", 0);
            }
                        
            if (this.nameListNode.options.length==0) {
                this.nameListNode.addOption({
                    label: "Names",
                    value: 0
                });
            
                this.nameListNode.set("value", 0);
            }
                        
            var options = [];
            
            var names = appConfig.layers[1].objects;
            for (i=0;i<names.length;i++) {
                var attributes = names[i].attributes;
                options[options.length] = {
                    label: this.createName(this.getHonorific(attributes[1].value), attributes[2].value, attributes[3].value, attributes[4].value),
                    value: attributes[0].value
                };
            }
            
        	// now add the old names
            var oldNames = this.nameSingleton.names.features;
                    	
            for (i=0;i<oldNames.length;i++) {
                var attributes = oldNames[i].attributes;
                options[options.length] = {
                    label: this.createName(this.getHonorific(attributes.Honorific), attributes.FirstName, attributes.MiddleName, attributes.LastName),
                    value: attributes.NAMEID_PK
                };
            }
            
            if (options.length>0) {
                this.nameListNode.addOption(options);
                this.nameListNode.set("value", options[0].value);
        	}
        	
        	var featureNames = appConfig.layers[2].objects;
        	var featureOptions = [];
        	var i;
        	for (i = 0 ; i<featureNames.length;i++){
        	    var attributesFN = featureNames[i].attributes;
        	    
        	    if (attributesFN[2].valueLabel===this.nameType && attributesFN[1].value==this.featureValue) {
        	        // find in name list
        	        var option = this.findNameCreateOption(attributesFN[0].value);
        	        
        	        if (option!=null) {
        	            featureOptions[featureOptions.length] = option;
        	        }
        	    }
        	}
        	
            if (featureOptions.length>0) {
                this.newNameListNode.addOption(featureOptions);
                this.newNameListNode.set("value", featureOptions[0].value);
        	}
        },
        
        addNewNameList: function (value, honorific, honorificLabel, fName, mName, lName, email) {
            // Unique remove the current option
            if (this.unique && this.newNameListNode.options.length>1) {
                this.removeNewNameList(this.newNameListNode.options[1].value);
            }
            
        	appConfig.layers[2] = addNameFeatureObject(appConfig.layers[2]);
    		indxF = appConfig.layers[2].objects.length-1;
    		var nfAttributes = appConfig.layers[2].objects[indxF].attributes;

            if (value==null) {
    		    appConfig.layers[1] = addNameObject(appConfig.layers[1]);
        	    nameIndx = appConfig.layers[1].objects.length-1;
        	    var attributes = appConfig.layers[1].objects[nameIndx].attributes;
    		
    		    // generate id for name
        	    attributes[0].value = appConfig.generateUUID();

        	    attributes[1].value = honorific;
        	    attributes[1].valueLabel = honorificLabel;
        	    attributes[2].value = fName;
        	    attributes[3].value = mName;
        	    attributes[4].value = lName;
         	    attributes[5].value = email;
         	    
         	    value = attributes[0].value;
       	    }
       	    
        	// cross link table name id and featureid
    		nfAttributes[0].value = value;
    		nfAttributes[1].value = this.featureValue;
    		nfAttributes[2].value = this.TYPE[this.nameType];
    		nfAttributes[2].valueLabel = this.nameType;

        	// create name
        	name = this.createName(honorificLabel, fName, mName, lName);
        	
        	// put in selection
        	var option = {
        		label: name,
        		value: value
        	}
        	
        	this.newNameListNode.addOption([option]);
        	this.newNameListNode.set("value", option.value);
        	
        	// clear name
        	this.clearName();
        },
        
        getIndexOfNameFromValue: function(value) {
            var index = -1;
            names = appConfig.layers[1].objects;
            for (i=0;i<names.length;i++) {
                var attributes = names[i].attributes;
                if (attributes[0].value==value) {
                    index = i;
                    break;
                }
            }
            
            return index;
        },
        
        getIndexOfFeatureNameFromValue: function(value) {
            var index = -1;
            names = appConfig.layers[2].objects;
            
            for (i=0;i<names.length;i++) {
                var attributes = names[i].attributes;
                if (attributes[0].value==value && attributes[1]==this.featureValue) {
                    index = i;
                    break;
                }
            }
            
            return index;
        },
        
        getIndexOfFeatureNameValue: function(value) {
            var index = -1;
            names = appConfig.layers[2].objects;
            
            for (i=0;i<names.length;i++) {
                var attributes = names[i].attributes;
                if (attributes[0].value==value) {
                    index = i;
                    break;
                }
            }
            
            return index;
        },
        
        removeNewNameList: function (value) {
            // remove name feature layer object
            appConfig.layers[2].objects.splice(this.getIndexOfFeatureNameFromValue(value),1);
            
            // remove name from layer if this is the only nameFeature
            if (this.getIndexOfFeatureNameValue(value)==-1) {
                // remove from name layer object
                appConfig.layers[1].objects.splice(this.getIndexOfNameFromValue(value),1);
            }
        
            // remove from list and select the first name
            this.newNameListNode.removeOption({ 
                    value: value 
            });
        
            this.newNameListNode.set("value", 0);
            
            if (this.mode=="edit") {
                this.editRemove[this.editRemove.length] = value;
                this.clearName();
            }
        },
        
        checkForName: function (value) {
        	var name = false;
        	
        	var featureNames = appConfig.layers[2].objects;
         	for (i=0;i<featureNames.length;i++) {
         	    var attributes = featureNames[i].attributes;
         	    if (attributes[1].value==value) {
         	        name = true;
         	        break;
         	    }
         	}
         	
        	return name;
        },
        
        getAttributes: function(value) {
            var names = appConfig.layers[1].objects;
            var attributes = null;
            for (i=0;i<names.length;i++) {
                attributes = names[i].attributes;
                if (attributes[0].value==value) {
                    break;
                } else {
                    attributes = null;
                }
            }
            
            return attributes;
        },

        getOldAttributes: function(value) {
            var attributes = null;
            var oldNames = this.nameSingleton.names.features;
                    
            for (i=0;i<oldNames.length;i++) {
                var oldAttributes = oldNames[i].attributes;
                if (oldAttributes.NAMEID_PK==value) {
                    attributes = [oldAttributes.NAMEID_PK, oldAttributes.Honorific, oldAttributes.FirstName, oldAttributes.MiddleName, oldAttributes.LastName, oldAttributes.Email];
                    break;
                }
            }
        
            return attributes;
        },

        newNameNumber: function () {
            count = 0;
            for (i=0;i<this.newNameListNode.options.length;i++) {
                if (this.newNameListNode.options[i].value!=0) {
                    count++;
                }
            }
            return count;
        },

        postCreate: function () {
            //make sure any parent widget's postCreate functions get called.
            this.inherited(arguments);
            
            this.updateNameNode.on("click", lang.hitch(this, function () {         
                // check if all are empty
                if	(this.lastNameNode.get("value")=="" && this.emailNode.get("value")=="" && this.nameListNode.get("value")==0) {
                    alert("Nothing to Update!");
                } else if (this.lastNameNode.get("value")=="") {
                            alert("A last name is required!");
                } else if (this.emailNode.get("value")=="") {
                            alert("An email address is required!");
                } else {
                    var value = this.newNameListNode.get("value");
                    
                    var attributes = this.getAttributes(value);
                    
                    if (attributes!=null) {
                        attributes[1].value = this.honorificNode.get("value");
                        attributes[2].value = this.firstNameNode.get("value");
                        attributes[3].value = this.middleNameNode.get("value");
                        attributes[4].value = this.lastNameNode.get("value");
                        attributes[5].value = this.emailNode.get("value");
                        
                       this.newNameListNode.updateOption({
                            label: this.createName(this.getHonorific(attributes[1].value), attributes[2].value, attributes[3].value, attributes[4].value),
                            value: this.newNameListNode.get("value")            
                        });
                    } else {
                        this.clearName();
                    }
                }
            }));

            this.addNameNode.on("click", lang.hitch(this, function () {
                
                // check if all are empty
                if	(this.lastNameNode.get("value")=="" && this.emailNode.get("value")=="" && this.nameListNode.get("value")==0) {
                    alert("Nothing to Add!");
                } else if (this.lastNameNode.get("value")=="" && this.emailNode.get("value")=="") { // use name list
                    var value = this.nameListNode.get("value");
                    
                    var attributes = this.getAttributes(value);
                                        
                    // put in name List
                    if (attributes!=null) {
                        this.addNewNameList(value, attributes[1].value, this.getHonorific(attributes[1].value), attributes[2].value, attributes[3].value, attributes[4].value, attributes[5].value);
                    } else {
                        attributes = this.getOldAttributes(value);
                        this.addNewNameList(value, attributes[1], this.getHonorific(attributes[1]), attributes[2], attributes[3], attributes[4], attributes[5]);
                    }
                } else if (this.lastNameNode.get("value")=="") {
                            alert("A last name is required!");
                } else if (this.emailNode.get("value")=="") {
                            alert("An email address is required!");
                } else {
                        // put in name List
                        this.addNewNameList(null, this.honorificNode.get("value"), this.getHonorific(this.honorificNode.get("value")), this.firstNameNode.get("value"), this.middleNameNode.get("value"), this.lastNameNode.get("value"), this.emailNode.get("value"));
                }

            }));
            
            this.removeNameNode.on("click", lang.hitch(this, function () {

            		    if (this.newNameListNode.get("value")!=0) {
            		        if (this.newNameNumber>1) {
                                // ask fer sher
                                if (confirm("Do you want to delete the selected name?")) {
                                    // remove from list
                                    this.removeNewNameList(this.newNameListNode.get("value"))
                                }
                            } else {
                                alert("Can't delete the only name!");
                            }
            		    } else {
            		    	    alert("Nothing to remove!");
            		    }
            }));
            
            this.newNameListNode.on("change", lang.hitch(this, function () {
                if (this.mode=="edit") {
                    var value = this.newNameListNode.get("value");
                    
                    var attributes = this.getAttributes(value);
                    
                    if (attributes!=null) {
                        this.honorificNode.set("value", attributes[1].value);
                        this.firstNameNode.set("value", attributes[2].value);
                        this.middleNameNode.set("value", attributes[3].value);
                        this.lastNameNode.set("value", attributes[4].value);
                        this.emailNode.set("value", attributes[5].value);
                    } else {
                        this.clearName();
                    }
                }
            }));
        }
    });
    
    ready(function(){
         console.log("what is happening?");
     });

});