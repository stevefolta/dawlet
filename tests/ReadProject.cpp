#include "../JSONParser.h"
#include "../Project.h"
#include "../Exception.h"
#include <stdio.h>
#include <string>

void TestReadProject()
{
	// Read the file into "text".
	const char* path = "tests/test-project.json";
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open %s!\n", path);
		return;
		}
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* contents = (char*) malloc(size);
	fread(contents, size, 1, file);
	fclose(file);
	std::string text(contents, contents + size);

	// Read the project.
	Project* project = new Project();
	try {
		JSONParser parser(text);
		project->read_json(&parser);
		printf("Project read succeded.\n");
		}
	catch (Exception& e) {
		fprintf(stderr, "Reading project failed: %s.\n", e.type.c_str());
		}
	delete project;

	// Clean up.
	free(contents);
}

