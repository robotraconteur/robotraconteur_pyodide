var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="beautifulsoup4.data";var REMOTE_PACKAGE_BASE="beautifulsoup4.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.8",true,true);Module["FS_createPath"]("/lib/python3.8","site-packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","beautifulsoup4-4.9.1-py3.8.egg-info",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","bs4",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/bs4","tests",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/bs4","builder",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:227687,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1222,2502,3782,5253,6269,7318,8508,9438,10441,11588,12351,13210,14105,14928,15786,16741,17791,18786,19704,20982,22180,23385,24572,25720,26862,27880,29152,30312,31488,32544,33647,34647,35681,36801,38027,39200,40254,41596,42812,43840,45002,46098,47524,48847,49957,50743,51525,52784,53931,55047,56188,57264,58373,59422,60486,61722,62770,64014,65223,66513,67765,69023,70029,71174,72505,73935,74973,75964,77396,78484,79544,80790,81955,83133,84253,85120,85842,86761,87484,88486,89620,90886,92287,93419,94770,96314,97895,99186,100001,101146,102233,103423,104688,105911,107111,108153,109208,110389,111518,112810,114127,115300,116313,117618,118916,120122,121275,122375,123341,124178,125393,126399,127481,128274,129292,130093,131054,131996,133119,133993,134959,135958,136826,137613,138737,139862,140814,141905,142873,143946,144703,145629,146555,147605,148764,149878,150809,151794,153126,154078,154749,155805,156992,157984,159132,160329,161655,162546,163434,164089,164804,165755,166626,167807,168936,170059,171249,172516,173689,174986,176107,176968,177803,178847,179967,181004,181760,182813,184122,185396,186408,187726,189090,190236,191479,192707,193721,194928,196352,197459,198813,199910,201090,202261,203255,204271,205365,206424,207287,208563,209667,211053,212253,213519,214859,215976,217451,218828,220120,221546,222728,224034,225218,226338,227543],sizes:[1222,1280,1280,1471,1016,1049,1190,930,1003,1147,763,859,895,823,858,955,1050,995,918,1278,1198,1205,1187,1148,1142,1018,1272,1160,1176,1056,1103,1e3,1034,1120,1226,1173,1054,1342,1216,1028,1162,1096,1426,1323,1110,786,782,1259,1147,1116,1141,1076,1109,1049,1064,1236,1048,1244,1209,1290,1252,1258,1006,1145,1331,1430,1038,991,1432,1088,1060,1246,1165,1178,1120,867,722,919,723,1002,1134,1266,1401,1132,1351,1544,1581,1291,815,1145,1087,1190,1265,1223,1200,1042,1055,1181,1129,1292,1317,1173,1013,1305,1298,1206,1153,1100,966,837,1215,1006,1082,793,1018,801,961,942,1123,874,966,999,868,787,1124,1125,952,1091,968,1073,757,926,926,1050,1159,1114,931,985,1332,952,671,1056,1187,992,1148,1197,1326,891,888,655,715,951,871,1181,1129,1123,1190,1267,1173,1297,1121,861,835,1044,1120,1037,756,1053,1309,1274,1012,1318,1364,1146,1243,1228,1014,1207,1424,1107,1354,1097,1180,1171,994,1016,1094,1059,863,1276,1104,1386,1200,1266,1340,1117,1475,1377,1292,1426,1182,1306,1184,1120,1205,144],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_beautifulsoup4.data")}Module["addRunDependency"]("datafile_beautifulsoup4.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{filename:"/lib/python3.8/site-packages/beautifulsoup4-4.9.1-py3.8.egg-info/SOURCES.txt",start:0,end:1124,audio:0},{filename:"/lib/python3.8/site-packages/beautifulsoup4-4.9.1-py3.8.egg-info/top_level.txt",start:1124,end:1128,audio:0},{filename:"/lib/python3.8/site-packages/beautifulsoup4-4.9.1-py3.8.egg-info/PKG-INFO",start:1128,end:5901,audio:0},{filename:"/lib/python3.8/site-packages/beautifulsoup4-4.9.1-py3.8.egg-info/requires.txt",start:5901,end:5949,audio:0},{filename:"/lib/python3.8/site-packages/beautifulsoup4-4.9.1-py3.8.egg-info/dependency_links.txt",start:5949,end:5950,audio:0},{filename:"/lib/python3.8/site-packages/bs4/element.py",start:5950,end:87016,audio:0},{filename:"/lib/python3.8/site-packages/bs4/testing.py",start:87016,end:131913,audio:0},{filename:"/lib/python3.8/site-packages/bs4/dammit.py",start:131913,end:166043,audio:0},{filename:"/lib/python3.8/site-packages/bs4/diagnose.py",start:166043,end:173798,audio:0},{filename:"/lib/python3.8/site-packages/bs4/__init__.py",start:173798,end:205349,audio:0},{filename:"/lib/python3.8/site-packages/bs4/formatter.py",start:205349,end:211003,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/test_docs.py",start:211003,end:212070,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/test_lxml.py",start:212070,end:216175,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/test_htmlparser.py",start:216175,end:220116,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/test_builder_registry.py",start:220116,end:225698,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/__init__.py",start:225698,end:225725,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/test_tree.py",start:225725,end:314713,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/test_html5lib.py",start:314713,end:321467,audio:0},{filename:"/lib/python3.8/site-packages/bs4/tests/test_soup.py",start:321467,end:350770,audio:0},{filename:"/lib/python3.8/site-packages/bs4/builder/_htmlparser.py",start:350770,end:369176,audio:0},{filename:"/lib/python3.8/site-packages/bs4/builder/_html5lib.py",start:369176,end:387924,audio:0},{filename:"/lib/python3.8/site-packages/bs4/builder/__init__.py",start:387924,end:407765,audio:0},{filename:"/lib/python3.8/site-packages/bs4/builder/_lxml.py",start:407765,end:419999,audio:0}],remote_package_size:231783,package_uuid:"28391ab3-563e-4a85-bd93-436c4b301cb6"})})();