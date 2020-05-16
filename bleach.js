var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};Module.checkABI(1);if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="bleach.data";var REMOTE_PACKAGE_BASE="bleach.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.7",true,true);Module["FS_createPath"]("/lib/python3.7","site-packages",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages","bleach",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach","_vendor",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach/_vendor","html5lib",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach/_vendor/html5lib","_trie",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach/_vendor/html5lib","filters",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach/_vendor/html5lib","treeadapters",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach/_vendor/html5lib","treebuilders",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach/_vendor/html5lib","treewalkers",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages/bleach/_vendor","html5lib-1.0.1.dist-info",true,true);Module["FS_createPath"]("/lib/python3.7/site-packages","bleach-3.1.0-py3.7.egg-info",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:300958,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1312,2691,3958,5007,6216,7197,8452,9706,10966,12174,13296,15070,16528,17754,19041,20148,21134,22146,23094,24209,25467,26768,28010,29079,30146,31242,32502,33723,34852,36049,37397,38819,40206,41456,42789,44076,45531,47069,48555,49505,50601,51722,52829,54171,55329,56499,57628,59019,60140,61051,62101,63120,64196,65122,66237,67168,68251,69571,70406,71603,72586,73643,74435,74950,75706,76498,77171,77769,78436,79050,79564,80205,80936,81408,82073,82925,83851,84523,85145,85875,86737,87620,88063,88626,89296,89905,90667,91428,91941,92533,93144,93704,94223,95125,96595,97753,98766,99499,100299,101050,101735,102373,103233,103832,104584,105709,106759,107895,109038,109921,110867,111757,112648,113556,114456,115398,116308,117234,118083,119008,119949,120869,121733,122667,123589,124490,125430,126206,127064,128005,128937,129874,130741,131641,132569,133421,134667,135678,136927,138014,138858,139947,141291,142367,143459,144428,144827,145495,146239,147031,147979,148957,149764,150701,151691,152611,153636,154675,155439,156298,156961,157834,158680,159522,160466,161389,162091,163096,164042,164879,165748,166750,167803,168537,169469,170367,171363,172259,173067,174014,174769,175756,176766,177663,178624,179297,180129,181072,181977,182894,183767,184594,185351,185988,187140,188342,189596,190783,191928,192932,193847,194645,195822,196731,197865,198890,199934,200576,201599,202465,203346,204202,204993,205845,206299,206796,207508,208226,208955,209726,210558,211636,212503,213671,214797,215901,217165,218335,219435,220860,222060,223212,224298,225422,226425,227531,228540,229763,230563,231558,232527,233329,234401,235335,236249,237081,238033,238928,240225,241119,242018,243094,243902,244909,246064,247474,248498,249282,250540,251501,252440,253429,254671,255588,256577,257488,258597,259736,260844,261979,263566,265104,266599,268193,269766,271261,272294,273428,275044,276543,278077,279634,281184,282625,284104,285481,286849,288079,289316,290348,291591,292584,293790,294944,296165,297303,298347,299462,300293],sizes:[1312,1379,1267,1049,1209,981,1255,1254,1260,1208,1122,1774,1458,1226,1287,1107,986,1012,948,1115,1258,1301,1242,1069,1067,1096,1260,1221,1129,1197,1348,1422,1387,1250,1333,1287,1455,1538,1486,950,1096,1121,1107,1342,1158,1170,1129,1391,1121,911,1050,1019,1076,926,1115,931,1083,1320,835,1197,983,1057,792,515,756,792,673,598,667,614,514,641,731,472,665,852,926,672,622,730,862,883,443,563,670,609,762,761,513,592,611,560,519,902,1470,1158,1013,733,800,751,685,638,860,599,752,1125,1050,1136,1143,883,946,890,891,908,900,942,910,926,849,925,941,920,864,934,922,901,940,776,858,941,932,937,867,900,928,852,1246,1011,1249,1087,844,1089,1344,1076,1092,969,399,668,744,792,948,978,807,937,990,920,1025,1039,764,859,663,873,846,842,944,923,702,1005,946,837,869,1002,1053,734,932,898,996,896,808,947,755,987,1010,897,961,673,832,943,905,917,873,827,757,637,1152,1202,1254,1187,1145,1004,915,798,1177,909,1134,1025,1044,642,1023,866,881,856,791,852,454,497,712,718,729,771,832,1078,867,1168,1126,1104,1264,1170,1100,1425,1200,1152,1086,1124,1003,1106,1009,1223,800,995,969,802,1072,934,914,832,952,895,1297,894,899,1076,808,1007,1155,1410,1024,784,1258,961,939,989,1242,917,989,911,1109,1139,1108,1135,1587,1538,1495,1594,1573,1495,1033,1134,1616,1499,1534,1557,1550,1441,1479,1377,1368,1230,1237,1032,1243,993,1206,1154,1221,1138,1044,1115,831,665],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_bleach.data")}Module["addRunDependency"]("datafile_bleach.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{start:0,audio:0,end:3775,filename:"/lib/python3.7/site-packages/bleach/__init__.py"},{start:3775,audio:0,end:4579,filename:"/lib/python3.7/site-packages/bleach/callbacks.py"},{start:4579,audio:0,end:22508,filename:"/lib/python3.7/site-packages/bleach/html5lib_shim.py"},{start:22508,audio:0,end:42104,filename:"/lib/python3.7/site-packages/bleach/linkifier.py"},{start:42104,audio:0,end:62986,filename:"/lib/python3.7/site-packages/bleach/sanitizer.py"},{start:62986,audio:0,end:64101,filename:"/lib/python3.7/site-packages/bleach/utils.py"},{start:64101,audio:0,end:65358,filename:"/lib/python3.7/site-packages/bleach/_vendor/README.rst"},{start:65358,audio:0,end:65358,filename:"/lib/python3.7/site-packages/bleach/_vendor/__init__.py"},{start:65358,audio:0,end:65448,filename:"/lib/python3.7/site-packages/bleach/_vendor/pip_install_vendor.sh"},{start:65448,audio:0,end:65634,filename:"/lib/python3.7/site-packages/bleach/_vendor/vendor.txt"},{start:65634,audio:0,end:66779,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/__init__.py"},{start:66779,audio:0,end:83484,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_ihatexml.py"},{start:83484,audio:0,end:115983,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_inputstream.py"},{start:115983,audio:0,end:192551,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_tokenizer.py"},{start:192551,audio:0,end:196554,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_utils.py"},{start:196554,audio:0,end:280072,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/constants.py"},{start:280072,audio:0,end:399023,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/html5parser.py"},{start:399023,audio:0,end:414769,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/serializer.py"},{start:414769,audio:0,end:415058,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_trie/__init__.py"},{start:415058,audio:0,end:415988,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_trie/_base.py"},{start:415988,audio:0,end:417154,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_trie/datrie.py"},{start:417154,audio:0,end:418917,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/_trie/py.py"},{start:418917,audio:0,end:418917,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/__init__.py"},{start:418917,audio:0,end:419836,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/alphabeticalattributes.py"},{start:419836,audio:0,end:420122,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/base.py"},{start:420122,audio:0,end:423067,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/inject_meta_charset.py"},{start:423067,audio:0,end:426698,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/lint.py"},{start:426698,audio:0,end:437286,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/optionaltags.py"},{start:437286,audio:0,end:463522,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/sanitizer.py"},{start:463522,audio:0,end:464736,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/filters/whitespace.py"},{start:464736,audio:0,end:465386,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treeadapters/__init__.py"},{start:465386,audio:0,end:467101,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treeadapters/genshi.py"},{start:467101,audio:0,end:468877,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treeadapters/sax.py"},{start:468877,audio:0,end:472469,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treebuilders/__init__.py"},{start:472469,audio:0,end:487036,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treebuilders/base.py"},{start:487036,audio:0,end:495871,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treebuilders/dom.py"},{start:495871,audio:0,end:508623,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treebuilders/etree.py"},{start:508623,audio:0,end:522745,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treebuilders/etree_lxml.py"},{start:522745,audio:0,end:528459,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treewalkers/__init__.py"},{start:528459,audio:0,end:535935,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treewalkers/base.py"},{start:535935,audio:0,end:537348,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treewalkers/dom.py"},{start:537348,audio:0,end:541886,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treewalkers/etree.py"},{start:541886,audio:0,end:548183,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treewalkers/etree_lxml.py"},{start:548183,audio:0,end:550492,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib/treewalkers/genshi.py"},{start:550492,audio:0,end:564255,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/DESCRIPTION.rst"},{start:564255,audio:0,end:564259,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/INSTALLER"},{start:564259,audio:0,end:565343,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/LICENSE.txt"},{start:565343,audio:0,end:580827,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/METADATA"},{start:580827,audio:0,end:584427,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/RECORD"},{start:584427,audio:0,end:584540,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/WHEEL"},{start:584540,audio:0,end:586271,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/metadata.json"},{start:586271,audio:0,end:586280,filename:"/lib/python3.7/site-packages/bleach/_vendor/html5lib-1.0.1.dist-info/top_level.txt"},{start:586280,audio:0,end:610933,filename:"/lib/python3.7/site-packages/bleach-3.1.0-py3.7.egg-info/PKG-INFO"},{start:610933,audio:0,end:614199,filename:"/lib/python3.7/site-packages/bleach-3.1.0-py3.7.egg-info/SOURCES.txt"},{start:614199,audio:0,end:614200,filename:"/lib/python3.7/site-packages/bleach-3.1.0-py3.7.egg-info/dependency_links.txt"},{start:614200,audio:0,end:614201,filename:"/lib/python3.7/site-packages/bleach-3.1.0-py3.7.egg-info/not-zip-safe"},{start:614201,audio:0,end:614225,filename:"/lib/python3.7/site-packages/bleach-3.1.0-py3.7.egg-info/requires.txt"},{start:614225,audio:0,end:614232,filename:"/lib/python3.7/site-packages/bleach-3.1.0-py3.7.egg-info/top_level.txt"}],remote_package_size:305054,package_uuid:"9e71fffd-6bc5-46b9-97c7-8273bb950948"})})();