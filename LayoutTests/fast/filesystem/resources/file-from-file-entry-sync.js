// FIXME: move this under workers/resources
importScripts('fs-worker-common.js');

description("Obtaining File from FileEntrySync.");

var fileSystem = requestFileSystemSync(this.TEMPORARY, 100);
removeAllInDirectorySync(fileSystem.root);

var testFileName = '/testFileEntry.txt';

var testFileEntry = fileSystem.root.getFile(testFileName, {create:true});
var testFile = testFileEntry.file();

shouldBe("testFile.name", "testFileEntry.name");
shouldBe("testFile.type", "'text/plain'");
shouldBe("testFile.size", "0");

removeAllInDirectorySync(fileSystem.root);
finishJSTest();
var successfullyParsed = true;
