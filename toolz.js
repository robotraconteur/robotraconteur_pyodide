var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};Module.checkABI(1);if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="toolz.data";var REMOTE_PACKAGE_BASE="toolz.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.7",true,true);Module["FS_createPath"]("/lib/python3.7","site-packages",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages","tlz",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages","toolz",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/toolz","curried",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/toolz","sandbox",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/toolz","tests",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages","toolz-0.10.0-py3.7.egg-info",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:99421,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1063,2241,3490,4302,5106,5965,6782,7210,7723,8658,9763,10728,11463,12625,13732,14726,15865,17125,18268,19403,20625,21583,22613,23806,25112,26242,27336,28706,30004,30960,31945,32574,33336,34196,35307,36655,37880,38900,39937,41085,42396,43611,44891,46291,47536,48694,49897,51104,52552,53660,54910,56320,57602,59063,60247,61326,62494,63200,64146,65198,66325,67271,68302,69327,70200,71164,72094,73159,74296,75420,76431,77021,77579,78310,79082,79809,80611,81815,82936,83901,84772,85665,86681,87633,88651,89380,90319,91404,92401,93215,94115,94672,95588,96685,97918,98954],sizes:[1063,1178,1249,812,804,859,817,428,513,935,1105,965,735,1162,1107,994,1139,1260,1143,1135,1222,958,1030,1193,1306,1130,1094,1370,1298,956,985,629,762,860,1111,1348,1225,1020,1037,1148,1311,1215,1280,1400,1245,1158,1203,1207,1448,1108,1250,1410,1282,1461,1184,1079,1168,706,946,1052,1127,946,1031,1025,873,964,930,1065,1137,1124,1011,590,558,731,772,727,802,1204,1121,965,871,893,1016,952,1018,729,939,1085,997,814,900,557,916,1097,1233,1036,467],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_toolz.data")}Module["addRunDependency"]("datafile_toolz.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{start:0,audio:0,end:338,filename:"/lib/python3.7/site-packages/tlz/__init__.py"},{start:338,audio:0,end:3685,filename:"/lib/python3.7/site-packages/tlz/_build_tlz.py"},{start:3685,audio:0,end:4009,filename:"/lib/python3.7/site-packages/toolz/__init__.py"},{start:4009,audio:0,end:26477,filename:"/lib/python3.7/site-packages/toolz/_signatures.py"},{start:26477,audio:0,end:27634,filename:"/lib/python3.7/site-packages/toolz/compatibility.py"},{start:27634,audio:0,end:36633,filename:"/lib/python3.7/site-packages/toolz/dicttoolz.py"},{start:36633,audio:0,end:71138,filename:"/lib/python3.7/site-packages/toolz/functoolz.py"},{start:71138,audio:0,end:98800,filename:"/lib/python3.7/site-packages/toolz/itertoolz.py"},{start:98800,audio:0,end:100087,filename:"/lib/python3.7/site-packages/toolz/recipes.py"},{start:100087,audio:0,end:100226,filename:"/lib/python3.7/site-packages/toolz/utils.py"},{start:100226,audio:0,end:102926,filename:"/lib/python3.7/site-packages/toolz/curried/__init__.py"},{start:102926,audio:0,end:103263,filename:"/lib/python3.7/site-packages/toolz/curried/exceptions.py"},{start:103263,audio:0,end:103753,filename:"/lib/python3.7/site-packages/toolz/curried/operator.py"},{start:103753,audio:0,end:103821,filename:"/lib/python3.7/site-packages/toolz/sandbox/__init__.py"},{start:103821,audio:0,end:108157,filename:"/lib/python3.7/site-packages/toolz/sandbox/core.py"},{start:108157,audio:0,end:110988,filename:"/lib/python3.7/site-packages/toolz/sandbox/parallel.py"},{start:110988,audio:0,end:111533,filename:"/lib/python3.7/site-packages/toolz/tests/test_compatibility.py"},{start:111533,audio:0,end:115180,filename:"/lib/python3.7/site-packages/toolz/tests/test_curried.py"},{start:115180,audio:0,end:115454,filename:"/lib/python3.7/site-packages/toolz/tests/test_curried_doctests.py"},{start:115454,audio:0,end:124387,filename:"/lib/python3.7/site-packages/toolz/tests/test_dicttoolz.py"},{start:124387,audio:0,end:144724,filename:"/lib/python3.7/site-packages/toolz/tests/test_functoolz.py"},{start:144724,audio:0,end:160940,filename:"/lib/python3.7/site-packages/toolz/tests/test_inspect_args.py"},{start:160940,audio:0,end:179143,filename:"/lib/python3.7/site-packages/toolz/tests/test_itertoolz.py"},{start:179143,audio:0,end:179963,filename:"/lib/python3.7/site-packages/toolz/tests/test_recipes.py"},{start:179963,audio:0,end:185821,filename:"/lib/python3.7/site-packages/toolz/tests/test_serialization.py"},{start:185821,audio:0,end:188748,filename:"/lib/python3.7/site-packages/toolz/tests/test_signatures.py"},{start:188748,audio:0,end:190257,filename:"/lib/python3.7/site-packages/toolz/tests/test_tlz.py"},{start:190257,audio:0,end:190413,filename:"/lib/python3.7/site-packages/toolz/tests/test_utils.py"},{start:190413,audio:0,end:196627,filename:"/lib/python3.7/site-packages/toolz-0.10.0-py3.7.egg-info/PKG-INFO"},{start:196627,audio:0,end:197528,filename:"/lib/python3.7/site-packages/toolz-0.10.0-py3.7.egg-info/SOURCES.txt"},{start:197528,audio:0,end:197529,filename:"/lib/python3.7/site-packages/toolz-0.10.0-py3.7.egg-info/dependency_links.txt"},{start:197529,audio:0,end:197530,filename:"/lib/python3.7/site-packages/toolz-0.10.0-py3.7.egg-info/not-zip-safe"},{start:197530,audio:0,end:197540,filename:"/lib/python3.7/site-packages/toolz-0.10.0-py3.7.egg-info/top_level.txt"}],remote_package_size:103517,package_uuid:"7a7113f2-b5f5-47ce-b39e-6ce47e9746bf"})})();