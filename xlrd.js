var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="xlrd.data";var REMOTE_PACKAGE_BASE="xlrd.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.8",true,true);Module["FS_createPath"]("/lib/python3.8","site-packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","xlrd",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","xlrd-1.2.0-py3.8.egg-info",true,true);Module["FS_createPath"]("/","bin",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:218989,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1403,2431,3770,5011,6264,7625,8920,10178,11238,12457,13407,14498,15796,16896,17957,19137,20189,21391,22525,23586,24667,25866,27014,27810,28879,29940,31130,32356,33848,35163,36471,37306,38585,39592,40790,41783,42528,43371,44410,45285,46417,47412,48409,49346,50186,51261,52118,53122,54166,55202,56245,57200,58235,59046,60027,61119,61871,62640,63550,64296,65458,66717,67726,68875,69947,71141,72196,73182,74401,75576,76747,78019,79273,80547,81767,83182,84289,85474,86709,87756,89029,90189,91357,92408,93510,94698,95529,96549,97606,98730,100104,101451,102656,103965,105293,106687,107955,109173,110580,112228,113329,114730,116018,117185,118477,119726,120555,121747,122888,124071,125117,126245,127414,128571,129685,130579,131345,132341,133434,134684,135598,136674,137366,138129,138908,139686,140401,141098,142003,142882,143747,144664,145792,147306,148533,149775,150978,151789,152409,153231,154298,155193,155992,157084,157957,158773,159728,160784,161991,163216,163983,164733,165812,166820,167903,168782,169756,170660,171697,172965,173762,174583,175630,176775,178e3,179348,180790,181932,183276,184363,185584,186930,188209,189136,190414,191779,192929,194138,195487,196700,197946,198952,199979,200915,202050,203274,204198,205226,206190,207486,208732,209780,211068,212150,213367,214465,215576,216587,217667,218562],sizes:[1403,1028,1339,1241,1253,1361,1295,1258,1060,1219,950,1091,1298,1100,1061,1180,1052,1202,1134,1061,1081,1199,1148,796,1069,1061,1190,1226,1492,1315,1308,835,1279,1007,1198,993,745,843,1039,875,1132,995,997,937,840,1075,857,1004,1044,1036,1043,955,1035,811,981,1092,752,769,910,746,1162,1259,1009,1149,1072,1194,1055,986,1219,1175,1171,1272,1254,1274,1220,1415,1107,1185,1235,1047,1273,1160,1168,1051,1102,1188,831,1020,1057,1124,1374,1347,1205,1309,1328,1394,1268,1218,1407,1648,1101,1401,1288,1167,1292,1249,829,1192,1141,1183,1046,1128,1169,1157,1114,894,766,996,1093,1250,914,1076,692,763,779,778,715,697,905,879,865,917,1128,1514,1227,1242,1203,811,620,822,1067,895,799,1092,873,816,955,1056,1207,1225,767,750,1079,1008,1083,879,974,904,1037,1268,797,821,1047,1145,1225,1348,1442,1142,1344,1087,1221,1346,1279,927,1278,1365,1150,1209,1349,1213,1246,1006,1027,936,1135,1224,924,1028,964,1296,1246,1048,1288,1082,1217,1098,1111,1011,1080,895,427],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_xlrd.data")}Module["addRunDependency"]("datafile_xlrd.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{filename:"/lib/python3.8/site-packages/xlrd/book.py",start:0,end:57073,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/sheet.py",start:57073,end:163219,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/compdoc.py",start:163219,end:184143,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/xldate.py",start:184143,end:192077,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/biffh.py",start:192077,end:208728,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/timemachine.py",start:208728,end:210485,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/xlsx.py",start:210485,end:244012,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/formula.py",start:244012,end:338467,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/__init__.py",start:338467,end:344958,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/info.py",start:344958,end:344994,audio:0},{filename:"/lib/python3.8/site-packages/xlrd/formatting.py",start:344994,end:390540,audio:0},{filename:"/lib/python3.8/site-packages/xlrd-1.2.0-py3.8.egg-info/SOURCES.txt",start:390540,end:392736,audio:0},{filename:"/lib/python3.8/site-packages/xlrd-1.2.0-py3.8.egg-info/top_level.txt",start:392736,end:392741,audio:0},{filename:"/lib/python3.8/site-packages/xlrd-1.2.0-py3.8.egg-info/PKG-INFO",start:392741,end:394039,audio:0},{filename:"/lib/python3.8/site-packages/xlrd-1.2.0-py3.8.egg-info/dependency_links.txt",start:394039,end:394040,audio:0},{filename:"/bin/runxlrd.py",start:394040,end:410330,audio:0}],remote_package_size:223085,package_uuid:"c538e8b9-26b4-4d3f-9291-fba3f64ece58"})})();