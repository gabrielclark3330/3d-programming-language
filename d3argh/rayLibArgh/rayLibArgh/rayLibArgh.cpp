#include <cstdio>
#include "raylib.h"
#include "rlgl.h"
#include <stddef.h>     // Required for: NULL
#include <math.h>       // Required for: sinf()
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <vector>

using namespace std;

// To make it work with the older RLGL module just comment the line below
#define RAYLIB_NEW_RLGL

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
#define LETTER_BOUNDRY_SIZE     0.25f
#define TEXT_MAX_LAYERS         32
#define LETTER_BOUNDRY_COLOR    VIOLET

bool SHOW_LETTER_BOUNDRY = false;
bool SHOW_TEXT_BOUNDRY = false;

enum Direction {Up, Down, Right, Left};
enum Mode {Insert, Normal, Visual};

//--------------------------------------------------------------------------------------
// Module Functions Declaration
//--------------------------------------------------------------------------------------
// Draw a codepoint in 3D space
void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint);
// Draw a 2D text in 3D space
void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint);
// Measure a text in 3D. For some reason `MeasureTextEx()` just doesn't seem to work so i had to use this instead.
Vector3 MeasureText3D(Font font, const char* text, float fontSize, float fontSpacing, float lineSpacing);

void moveCamOnSphere(Camera3D& camera, Direction direction, float amount,
    float& angleAroundZ, float& angleUpDown, float& distanceFromTarget, Vector3 codeCubeSize);
// Generates a nice color with a random hue
Color GenerateRandomColor(float s, float v);

void interpret(char(&state)[80][80][80], Vector3& instructionPointer, Vector3& instructionDirection, vector<char>& stack, Vector3 codeCubeSize, vector<char> &vecOut);

void drawTextRotatedToCam(char* text, Vector3 pos, float phi, float theta) {
	char* opt = (char*)text;
	Vector3 m = MeasureText3D(GetFontDefault(), opt, 8.0f, 1.0f, 0.0f);
	rlPushMatrix();
	rlTranslatef(pos.x, pos.y, pos.z);
	rlRotatef(phi*180/PI, 0.0f, -1.0f, 0.0f);
	rlRotatef(theta*180/PI, 0.0f, 0.0f, -1.0f);
	rlRotatef(90, 0.0f, -1.0f, 0.0f);
	rlRotatef(180, 1.0f, 0.0f, 0.0f);
    Vector3 zero = { 0,0,0 };
	DrawText3D(GetFontDefault(), opt, zero, 10.0f, 0.5f, 0.0f, true, DARKBLUE);
	rlPopMatrix();
}

void loadState(char(&state)[80][80][80], int screenWidth, int screenHeight, Vector3 &stateSize, char* droppedFile) {
	ifstream readFile;
	readFile.open(droppedFile); // , ios::out | ios::trunc | ios::app
	string line;
	if (readFile)
	{
        Vector3 size = {};
        getline(readFile, line); // Get rid of the "Size()" line above the size numbers
		if (getline(readFile, line)) { // parse the size of the loading state
            // Size() indexes for x, y, z
            // num1,num2,num3

            int commaOne = line.find(",");
            std::string afterCommaOne = line.substr(commaOne+1, line.size());
            int commaTwo = afterCommaOne.find(",") + commaOne+1;

            std::string numOne = line.substr(0, commaOne);
            std::string numTwo = line.substr(commaOne+1, commaTwo-commaOne-1);
            std::string numThree = line.substr(commaTwo+1, line.size()-commaTwo-1);
            size.x = std::stoi(numOne);
            size.y = std::stoi(numTwo);
            size.z = std::stoi(numThree);
            stateSize.x = size.x;
            stateSize.y = size.y;
            stateSize.z = size.z;
		}
		else {
            printf("[ERROR] parsing");
		}
        int x = 0;
        int y = 0;
        int z = 0;
        for (z = 0; z < size.z; z++) {
            for (y = size.y-1; y >= 0; y--) {
                getline(readFile, line);
                int lineIndex = 0;
				for (x = size.x-1; x >= 0; x--) {
					state[x][y][z] = line.at(lineIndex);
                    lineIndex += 1;
				}
            }
            getline(readFile, line); // gets rid of ----- line
        }
		readFile.close();
	}

	/*
	string xyBreak = "";
	for (int i = 0; i <= stateSize.x; i++) {
		xyBreak.append("-");
	}
	string sizeString = "Size(" + std::to_string((int)stateSize.x) + "," + std::to_string((int)stateSize.y) + "," + std::to_string((int)stateSize.z) + ")";
	writeFile << sizeString << std::endl;
	for (float z = 0; z < stateSize.z; z++) {
		for (float y = stateSize.y-1; y >= 0; y--) {
			for (float x = stateSize.x-1; x >= 0; x--) {
				writeFile << state[(int)x][(int)y][(int)z];
			}
			writeFile << "," << std::endl;
		}
		writeFile << xyBreak << std::endl;
	}
    */
}

void saveStatePrompt(char(&state)[80][80][80], int screenWidth, int screenHeight, Vector3 stateSize) {
    string path = "";
    char charEntered = ' ';
    while (true) {
		BeginDrawing();
		ClearBackground(RAYWHITE);
        // draw prompt
		char* msg = (char*)"Enter absolute path to save code cube and press enter: ";
		int width = MeasureText(msg, 40);
		DrawText(msg, screenWidth/2 - width/2, screenHeight/2, 40, DARKGREEN);

        // draw file path as its entered
		msg = const_cast<char*>(path.c_str());
		width = MeasureText(msg, 40);
		DrawText(msg, screenWidth/2 - width/2, screenHeight/2+40, 40, DARKGREEN);
		EndDrawing();

        // update state for next draw
		charEntered = GetCharPressed();
        if (charEntered) {
			path.push_back(charEntered);
        }
        if (IsKeyPressed(KEY_ENTER)) {
            ofstream writeFile;
            writeFile.open(path); // , ios::out | ios::trunc | ios::app
            string xyBreak = "";
            for (int i = 0; i <= stateSize.x; i++) {
				xyBreak.append("-");
            }
            string sizeString = "Size()";
            writeFile << sizeString << std::endl;
            writeFile << std::to_string((int)stateSize.x) + "," + std::to_string((int)stateSize.y) + "," + std::to_string((int)stateSize.z) << std::endl;
			for (float z = 0; z < stateSize.z; z++) {
				for (float y = stateSize.y-1; y >= 0; y--) {
                    for (float x = stateSize.x-1; x >= 0; x--) {
						writeFile << state[(int)x][(int)y][(int)z];
					}
                    writeFile << "," << std::endl;
				}
                writeFile << xyBreak << std::endl;
			}
            writeFile.close();
            break;
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (path.size() > 0) {
				path.pop_back();
            }
        }
    }
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1800;
    int screenHeight = 1200;
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "ARGH! 3D!");
    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = { 10.0f, 10.0f, 10.0f };   // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera mode type
    float theta = 1.7799999;
    float phi = 1.3399999;
    float distanceFromTarget = -22.500;
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    // Use the default font
    Font font = GetFontDefault();
    float fontSize = 8.0f;
    float fontSpacing = 0.5f;
    float lineSpacing = -1.0f;
    
    // Code cube info
    Mode mode = Normal;
    Vector3 stateSize = { 3,3,3 };
    Vector3 instructionPointer = { stateSize.x-1,stateSize.y-1,0 };
    Vector3 instructionDirection = { -1,0,0 }; // Code flow direction starts out going right at position {top, left, front}
    Vector3 codeCubeVisualIndex = { 0,0,0 };
    vector<char> stack = {}; // Queue
    vector<char> vecOut = {};

    static char state[80][80][80];
    for (float x = 0; x < stateSize.x; x++) {
        for (float y = 0; y < stateSize.y; y++) {
            for (float z = 0; z < stateSize.z; z++) {
                state[(int)x][(int)y][(int)z] = ' ';
            }
        }
    }
    Vector3 tbox = { 0 };
    int layers = 1;
    int quads = 0;
    float layerDistance = 0.01f;
    float time = 0.0f;

    // Setup a light and dark color
    Color light = CLITERAL(Color) {38, 70, 83, 255}; // dark blue
    Color dark = CLITERAL(Color) {233, 196, 106, 50}; // yellow

    // Load the alpha discard shader
    Shader alphaDiscard = LoadShader(NULL, "resources/shaders/glsl330/alpha_discard.fs");

    // Array filled with multiple random colors (when multicolor mode is set)
    Color multi[TEXT_MAX_LAYERS] = { 0 };
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
		screenWidth = GetScreenWidth();
		screenHeight = GetScreenHeight();
        // Update
        //----------------------------------------------------------------------------------
        // Handle font files dropped
        if (IsFileDropped())
        {
            int count = 0;
            char** droppedFiles = GetDroppedFiles(&count);

            // NOTE: We only support first ttf file dropped
            if (IsFileExtension(droppedFiles[0], ".ttf"))
            {
                UnloadFont(font);
                font = LoadFontEx(droppedFiles[0], fontSize, 0, 0);
            }
            else if (IsFileExtension(droppedFiles[0], ".fnt"))
            {
                UnloadFont(font);
                font = LoadFont(droppedFiles[0]);
                fontSize = font.baseSize;
            }
            else if (IsFileExtension(droppedFiles[0], ".txt")) {
                //std::cout << droppedFiles[0];
                loadState(state, screenWidth, screenHeight, stateSize, droppedFiles[0]);
            }
            ClearDroppedFiles();
        }

        /*
        // Handle clicking the cube
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Ray ray = GetMouseRay(GetMousePosition(), camera);

            // Check collision between ray and box
			RayCollision collision = GetRayCollisionBox(ray,
			{
                {
                cubePosition.x - cubeSize.x / 2, cubePosition.y - cubeSize.y / 2, cubePosition.z - cubeSize.z / 2
				},
				{
				cubePosition.x + cubeSize.x / 2, cubePosition.y + cubeSize.y / 2, cubePosition.z + cubeSize.z / 2
                }
            });
            if (collision.hit)
            {
                // Generate new random colors
                light = GenerateRandomColor(0.5f, 0.78f);
                dark = GenerateRandomColor(0.4f, 0.58f);
            }
        }*/


        /*
        // Handle text changes
        if (IsKeyPressed(KEY_LEFT)) fontSize -= 0.5f;
        else if (IsKeyPressed(KEY_RIGHT)) fontSize += 0.5f;
        else if (IsKeyPressed(KEY_UP)) fontSpacing -= 0.1f;
        else if (IsKeyPressed(KEY_DOWN)) fontSpacing += 0.1f;
        */
        // key assignment/bindings
        if (mode == Normal) {
			if (IsKeyDown(KEY_LEFT)) moveCamOnSphere(camera, Left, 0.02, theta, phi, distanceFromTarget, stateSize);
			else if (IsKeyDown(KEY_RIGHT)) moveCamOnSphere(camera, Right, 0.02, theta, phi, distanceFromTarget, stateSize);
			else if (IsKeyDown(KEY_UP)) moveCamOnSphere(camera, Up, 0.02, theta, phi, distanceFromTarget, stateSize);
			else if (IsKeyDown(KEY_DOWN)) moveCamOnSphere(camera, Down, 0.02, theta, phi, distanceFromTarget, stateSize);
			else if (IsKeyDown(KEY_O)) {
				distanceFromTarget -= 0.5;
				moveCamOnSphere(camera, Down, 0.0, theta, phi, distanceFromTarget, stateSize);
			}
			else if (IsKeyDown(KEY_P)) {
				distanceFromTarget += 0.5;
				moveCamOnSphere(camera, Down, 0.0, theta, phi, distanceFromTarget, stateSize);
            }
            else if (IsKeyPressed(KEY_H)) {
                if (instructionPointer.x < stateSize.x-1) {
					instructionPointer.x += 1;
                }
            }
            else if (IsKeyPressed(KEY_J)) {
                if (instructionPointer.y > 0) {
					instructionPointer.y -= 1;
                }
            }
            else if (IsKeyPressed(KEY_K)) {
                if (instructionPointer.y < stateSize.y-1) {
					instructionPointer.y += 1;
                }
            }
            else if (IsKeyPressed(KEY_L)) {
                if (instructionPointer.x > 0) {
					instructionPointer.x -= 1;
                }
            }
            else if (IsKeyPressed(KEY_W)) {
                if (instructionPointer.z > 0) {
					instructionPointer.z -= 1;
                }
            }
            else if (IsKeyPressed(KEY_E)) {
                if (instructionPointer.z < stateSize.z-1) {
					instructionPointer.z += 1;
                }
            }
            else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyDown(KEY_ENTER)) {
                interpret(state, instructionPointer, instructionDirection, stack, stateSize, vecOut);
            }
            else if (IsKeyPressed(KEY_MINUS)) {
                if (stateSize.x > 0) {
					stateSize.x -= 1;
					stateSize.y -= 1;
					stateSize.z -= 1;
                }
            }
            else if (IsKeyPressed(KEY_EQUAL)) { // Equal and plus button should be the same on almost all computers
                if (stateSize.x < 80) {
					stateSize.x += 1;
					stateSize.y += 1;
					stateSize.z += 1;
                }
            }
            else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyDown(KEY_S)) {
                saveStatePrompt(state, screenWidth, screenHeight, stateSize);
            }
			else if (IsKeyPressed(KEY_I)) {
				mode = Insert;
			}
        }
        else if (mode == Insert) {
		    // Handle text input
            int ch = GetCharPressed();
			if (IsKeyPressed(KEY_BACKSPACE))
			{
				// Remove last char
                int x = instructionPointer.x;
                int y = instructionPointer.y;
                int z = instructionPointer.z;
                state[x][y][z] = ' ';
			}
			else if (IsKeyPressed(KEY_ENTER))
			{
                if (instructionPointer.y>0) {
					instructionPointer.y -= 1;
                }
			}
			else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_BRACKET)) {
				mode = Normal;
			}
			else
            {
                if (ch) {
					printf("%i, %i, %i\n", (int)instructionPointer.x, (int)instructionPointer.y, (int)instructionPointer.z);
					state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z] = ch;
                    if (instructionPointer.x > 0) {
                        instructionPointer.x -= 1;
                    }else if (instructionPointer.x == 0 && instructionPointer.y > 0) {
                        instructionPointer.y -= 1;
                        instructionPointer.x = stateSize.x - 1;
                    }
                }
			}
        }
        else if (mode == Visual) {

        }

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginShaderMode(alphaDiscard);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        Vector3 cubeAt = {0,0,0};

        // Build 3d grid cube
        for (float x = 0; x < stateSize.x; x++) {
			for (float y = 0; y < stateSize.y; y++) {
				for (float z = 0; z < stateSize.z; z++) {
					Vector3 cubePosition = {(x*2)-stateSize.x, (y*2)-stateSize.y, (z*2)-stateSize.z};
					DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, light);
                    if (instructionPointer.x==x && instructionPointer.y==y && instructionPointer.z==z) {
                        cubeAt = cubePosition;
                    }
                    char ch = state[(int)x][(int)y][(int)z];
                    if (ch) {
                        //printf("%i\n", ch);
                        char chp[2];
                        chp[0] = ch;
                        chp[1] = '\0';
                        
						drawTextRotatedToCam((char*)chp, cubePosition, phi, theta);
                    }
				}
			}
        }
		DrawCube(cubeAt, 2.0f, 2.0f, 2.0f, dark);

        // Use a shader to handle the depth buffer issue with transparent textures
        // NOTE: more info at https://bedroomcoders.co.uk/raylib-billboards-advanced-use/
        rlPushMatrix();

        //----------------------------------------------------------------------------------
        //Draw Axis labels
        //----------------------------------------------------------------------------------
        char* opt = (char*)"h";
        Vector3 pos = {stateSize.x+5, 0.0f, 0.0f };
		drawTextRotatedToCam(opt, pos, phi, theta);
        opt = (char*)"l";
        pos = {-stateSize.x-5, 0.0f, 0.0f };
		drawTextRotatedToCam(opt, pos, phi, theta);
        opt = (char*)"k";
        pos = { 1.0f, stateSize.y+5, 0.0f };
		drawTextRotatedToCam(opt, pos, phi, theta);
        opt = (char*)"j";
        pos = { 1.0f, -stateSize.y-5, 0.0f };
		drawTextRotatedToCam(opt, pos, phi, theta);
        opt = (char*)"e";
        pos = { 1.0f, 0.0f, stateSize.z+5 };
		drawTextRotatedToCam(opt, pos, phi, theta);
        opt = (char*)"w";
        pos = { 1.0f, 0.0f, -stateSize.z-5 };
		drawTextRotatedToCam(opt, pos, phi, theta);

        rlPopMatrix();
        EndShaderMode();
        EndMode3D();

        //----------------------------------------------------------------------------------
        //Render Queue
        //----------------------------------------------------------------------------------
        char* msg = (char*)"Stack: ";
		int width = MeasureText(msg, 40);
		DrawText(msg, screenWidth/2 - width-20, 10, 40, DARKGREEN);
        int totalWidth = 0;
        for (int i = 0; i < stack.size(); i++) {
            char tmp[3];
			tmp[0] = stack[i];
            tmp[1] = ',';
            tmp[2] = '\0';
            int width = MeasureText(tmp, 40);
            DrawText(tmp, screenWidth/2+totalWidth, 10, 40, DARKGREEN);
            totalWidth += width+4;
        }

        //----------------------------------------------------------------------------------
        //Render Standard Out
        //----------------------------------------------------------------------------------
        msg = (char*)"Standard Out: ";
		width = MeasureText(msg, 40);
		DrawText(msg, screenWidth/2 - width-20, 60, 40, DARKGREEN);
        totalWidth = 0;
        for (int i = 0; i < vecOut.size(); i++) {
            char tmp[3];
			tmp[0] = vecOut[i];
            tmp[1] = ',';
            tmp[2] = '\0';
            int width = MeasureText(tmp, 40);
            DrawText(tmp, screenWidth/2+totalWidth, 60, 40, DARKGREEN);
            totalWidth += width+4;
        }


        DrawFPS(10, 10);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(font);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


//--------------------------------------------------------------------------------------
// Module Functions Definitions
//--------------------------------------------------------------------------------------
// Draw codepoint at specified position in 3D space
void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize / (float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding) / (float)font.baseSize * scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding) / (float)font.baseSize * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f * font.glyphPadding, font.recs[index].height + 2.0f * font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;
    float height = (float)(font.recs[index].height + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;

        if (SHOW_LETTER_BOUNDRY) DrawCubeWiresV({ position.x + width / 2, position.y, position.z + height / 2 }, { width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);

        rlCheckRenderBatchLimit(4 + 4 * backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Front Face
        rlNormal3f(0.0f, 1.0f, 0.0f);                                   // Normal Pointing Up
        rlTexCoord2f(tx, ty); rlVertex3f(x, y, z);              // Top Left Of The Texture and Quad
        rlTexCoord2f(tx, th); rlVertex3f(x, y, z + height);     // Bottom Left Of The Texture and Quad
        rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);     // Bottom Right Of The Texture and Quad
        rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);              // Top Right Of The Texture and Quad

        if (backface)
        {
            // Back Face
            rlNormal3f(0.0f, -1.0f, 0.0f);                              // Normal Pointing Down
            rlTexCoord2f(tx, ty); rlVertex3f(x, y, z);          // Top Right Of The Texture and Quad
            rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);          // Top Left Of The Texture and Quad
            rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
            rlTexCoord2f(tx, th); rlVertex3f(x, y, z + height); // Bottom Right Of The Texture and Quad
        }
        rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    int length = TextLength(text);          // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;               // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;               // Offset X to next character to draw

    float scale = fontSize / (float)font.baseSize;

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing / (float)font.baseSize * scale;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint3D(font, codepoint, { position.x + textOffsetX, position.y, position.z + textOffsetY }, fontSize, backface, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += (float)(font.recs[index].width + fontSpacing) / (float)font.baseSize * scale;
            else textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing) / (float)font.baseSize * scale;
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

Vector3 MeasureText3D(Font font, const char* text, float fontSize, float fontSpacing, float lineSpacing)
{
    int len = TextLength(text);
    int tempLen = 0;                // Used to count longer text line num chars
    int lenCounter = 0;

    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float scale = fontSize / (float)font.baseSize;
    float textHeight = scale;
    float textWidth = 0.0f;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < len; i++)
    {
        lenCounter++;

        int next = 0;
        letter = GetCodepoint(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            if (font.glyphs[index].advanceX != 0) textWidth += (font.glyphs[index].advanceX + fontSpacing) / (float)font.baseSize * scale;
            else textWidth += (font.recs[index].width + font.glyphs[index].offsetX) / (float)font.baseSize * scale;
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0.0f;
            textHeight += scale + lineSpacing / (float)font.baseSize * scale;
        }

        if (tempLen < lenCounter) tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    Vector3 vec = { 0 };
    vec.x = tempTextWidth + (float)((tempLen - 1) * fontSpacing / (float)font.baseSize * scale); // Adds chars spacing to measure
    vec.y = 0.25f;
    vec.z = textHeight;

    return vec;
}

void moveCamOnSphere(Camera3D &camera, Direction direction, float amount,
                          float &theta, float &phi, float &r, Vector3 codeCubeSize) {
    Vector3 position;
    if (direction == Up) {
        theta += amount;
    }
    else if (direction == Down) {
        theta -= amount;
    }
    else if (direction == Right) {
        phi += amount;
    }
    else if (direction == Left) {
        phi -= amount;
    }
    
    // Calculations from a rotation matrix in non matrix form
    position.x = r * cos(phi) * sin(theta);
    position.z = r * sin(phi) * sin(theta);
    position.y = r * cos(theta);
    camera.position = position;
    
    /*
    camera.position.x += codeCubeSize.x;
    camera.position.y += codeCubeSize.y;
    camera.position.z += codeCubeSize.z;
    */
    printf("%f, %f, %f \n", camera.position.x, camera.position.y, camera.position.z);
}

Color GenerateRandomColor(float s, float v)
{
    const float Phi = 0.618033988749895f; // Golden ratio conjugate
    float h = GetRandomValue(0, 360);
    h = fmodf((h + h * Phi), 360.0f);
    return ColorFromHSV(h, s, v);
}

void moveInstructionPointerForward(Vector3 &instructionPointer, Vector3 instructionDirection, Vector3 codeCubeSize) {
    if (instructionPointer.x+1 < codeCubeSize.x && instructionDirection.x > 0) {
		instructionPointer.x += instructionDirection.x;
    }
    else if (instructionPointer.x-1 >= 0 && instructionDirection.x < 0) {
		instructionPointer.x += instructionDirection.x;
    }
    if (instructionPointer.y+1 < codeCubeSize.y && instructionDirection.y > 0) {
		instructionPointer.y += instructionDirection.y;
    }
    else if (instructionPointer.y-1 >= 0 && instructionDirection.y < 0) {
		instructionPointer.y += instructionDirection.y;
    }
    if (instructionPointer.z+1 < codeCubeSize.z && instructionDirection.z > 0) {
		instructionPointer.z += instructionDirection.z;
    }
    else if (instructionPointer.z-1 >= 0 && instructionDirection.z < 0) {
		instructionPointer.z += instructionDirection.z;
    }
}

void rotateInstructionDirection(bool clockWise, Vector3 &instructionDirection) {
    if (clockWise) {
        if (instructionDirection.x == -1 && instructionDirection.y == 0) { // left -> up
            instructionDirection.x = 0;
            instructionDirection.y = 1;
        }
        else if (instructionDirection.x == 0 && instructionDirection.y == -1) { // down -> left 
            instructionDirection.x = -1;
            instructionDirection.y = -0;
        }
        else if (instructionDirection.x == 1 && instructionDirection.y == 0) { // right -> down
            instructionDirection.x = 0;
            instructionDirection.y = -1;
        }
        else if (instructionDirection.x == 0 && instructionDirection.y == 1) { // right -> down
            instructionDirection.x = 1;
            instructionDirection.y = 0;
        }
        // up <- left  (0,1) <- (-1, 0)
        // left <- down (-1, 0) <- (0, -1)
        // down <- right (0, -1) <- (1, 0)
        // right <- up (1, 0) <- (0, 1)
    }
    else {
        if (instructionDirection.x == 0 && instructionDirection.y == 1) { // up -> left
            instructionDirection.x = -1;
            instructionDirection.y = 0;
        }
        else if (instructionDirection.x == -1 && instructionDirection.y == 0) { // left -> down
            instructionDirection.x = 0;
            instructionDirection.y = -1;
        }
        else if (instructionDirection.x == 0 && instructionDirection.y == -1) { // down -> right
            instructionDirection.x = 1;
            instructionDirection.y = 0;
        }
        else if (instructionDirection.x == 1 && instructionDirection.y == 0) { // down -> right
            instructionDirection.x = 0;
            instructionDirection.y = 1;
        }
        // up -> left  (0,1) -> (-1, 0)
        // left -> down (-1, 0) -> (0, -1)
        // down -> right (0, -1) -> (1, 0)
        // right -> up (1, 0) -> (0, 1)
    }
}

// interpret() takes in the current state of the language and moves the instructionPointer one step forward.
// It will return true while the program is still running and false when finished.
void interpret(char (&state)[80][80][80], Vector3 &instructionPointer, Vector3 &instructionDirection, vector<char> &stack, Vector3 stateSize, vector<char> &vecOut) {
    char currentInstruction = state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z];

    /*
    bool printable = true;
    if (currentInstruction != printable) {

    }
    */

    // y++ is up   y-- is down
    // x++ is left x-- is right
    // z-- is front z++ is back
    // Back of stack is top
    switch (currentInstruction) {
		case 'A': // add value of the cell above the current cell to the value on top of the stack(see "R")
            if (instructionPointer.y+1 < stateSize.y) {
				stack.back() += state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z];
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'a': // add value of the cell below the current cell to the value on top of the stack
            if (instructionPointer.y-1 >= 0) {
				stack.back() += state[(int)instructionPointer.x][(int)instructionPointer.y-1][(int)instructionPointer.z];
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'D': // delete top value off stack
            if (stack.size() > 0) {
				stack.pop_back();
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'd': // duplicate top value on stack
            if (stack.size() > 0) {
				char topOfStack = stack.back();
				stack.push_back(topOfStack);
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
        /*
        // I don't want to implement EOF because idk what to use it for and e is a movement key
		case 'E': // insert value of system EOF in cell above
            break;
		case 'e': // insert value of system EOF in cell below
            break;
        */
		case 'F': // fetch(pop) value from top of stack and store to cell above
            if (instructionPointer.y+1 < stateSize.y) {
				if (stack.size() > 0) {
					char topOfStack = stack.back();
                    stack.pop_back();
                    state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z] = topOfStack;
				}
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'f': // fetch(pop) value from top of stack and store to cell below
            if (instructionPointer.y-1 >= 0) {
				if (stack.size() > 0) {
					char topOfStack = stack.back();
                    stack.pop_back();
                    state[(int)instructionPointer.x][(int)instructionPointer.y-1][(int)instructionPointer.z] = topOfStack;
				}
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'G': // get one byte from stdinand store in cell above
            char character;
            std::cin.get(character);
            if (instructionPointer.y+1 < stateSize.y) {
				state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z] = character;
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'g': // get one byte from stdinand store in cell below
            char bitIn;
            std::cin.get(bitIn);
            if (instructionPointer.y-1 >= 0) {
				state[(int)instructionPointer.x][(int)instructionPointer.y-1][(int)instructionPointer.z] = bitIn;
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'H': // jump(move instruction pointer) left to the next cell whose value matches the value on top of the stack, and set execution direction to "left"
            while (instructionPointer.x+1 < stateSize.x && state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z] != stack.back()) {
                instructionPointer.x += 1;
            }
            instructionDirection.x = 1;
            instructionDirection.y = 0;
            instructionDirection.z = 0;
            break;
		case 'h': // set execution direction to "left"
            instructionDirection.x = 1;
            instructionDirection.y = 0;
            instructionDirection.z = 0;
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'J': // jump down to the next cell whose value matches the value on top of the stack, and set execution direction to "down"
            while (instructionPointer.y > 0 && state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z] != stack.back()) {
                instructionPointer.y -= 1;
            }
            instructionDirection.x = 0;
            instructionDirection.y = -1;
            instructionDirection.z = 0;
            break;
		case 'j': // set execution direction to "down"
            instructionDirection.x = 0;
            instructionDirection.y = -1;
            instructionDirection.z = 0;
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'K': // jump up to the next cell whose value matches the value on top of the stack, and set execution direction to "up"
            while (instructionPointer.y+1 < stateSize.y && state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z] != stack.back()) {
                instructionPointer.y += 1;
            }
            instructionDirection.x = 0;
            instructionDirection.y = 1;
            instructionDirection.z = 0;
            break;
		case 'k': // set execution direction to "up"
            instructionDirection.x = 0;
            instructionDirection.y = 1;
            instructionDirection.z = 0;
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'L': // jump right to the next cell whose value matches the value on top of the stack, and set execution direction to "right"
            while (instructionPointer.x > 0 && state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z] != stack.back()) {
                instructionPointer.x -= 1;
            }
            instructionDirection.x = -1;
            instructionDirection.y = 0;
            instructionDirection.z = 0;
            break;
		case 'l': // set execution direction to "right"
            instructionDirection.x = -1;
            instructionDirection.y = 0;
            instructionDirection.z = 0;
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'W': // jump z+ to the next cell whose value matches the value on top of the stack, and set execution direction to "z+"
            while (instructionPointer.z > 0 && state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z] != stack.back()) {
                instructionPointer.z -= 1;
            }
            instructionDirection.x = 0;
            instructionDirection.y = 0;
            instructionDirection.z = -1;
            break;
		case 'w': // set execution direction to "z+"
            instructionDirection.x = 0;
            instructionDirection.y = 0;
            instructionDirection.z = -1;
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'E': // jump right to the next cell whose value matches the value on top of the stack, and set execution direction to "z-"
            while (instructionPointer.z+1 < stateSize.z && state[(int)instructionPointer.x][(int)instructionPointer.y][(int)instructionPointer.z] != stack.back()) {
                instructionPointer.z += 1;
            }
            instructionDirection.x = 0;
            instructionDirection.y = 0;
            instructionDirection.z = 1;
            break;
		case 'e': // set execution direction to "z-"
            instructionDirection.x = 0;
            instructionDirection.y = 0;
            instructionDirection.z = 1;
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'P': // send value above the current cell to stdout(does not remove the value[citation needed])
			printf("to out: %c \n", state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z]);
            if (instructionPointer.y+1 < stateSize.y) {
                printf("to out: %c\n", state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z]);
				vecOut.push_back(state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z]);
                printf("to out: %c\n", state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z]);
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'p': // send value below the current cell to stdout
            if (instructionPointer.y-1 >= 0) {
				vecOut.push_back(state[(int)instructionPointer.x][(int)instructionPointer.y-1][(int)instructionPointer.z]);
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'q': // quit: end program execution
            break;
		case 'R': // reduce the value on top of the stack by the value of the cell above(see "A")
            if (instructionPointer.y+1 < stateSize.y) {
				stack.back() -= state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z];
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
        case 'r': // reduce the value on top of the stack by the value of the cell below
            if (instructionPointer.y-1 >= 0) {
				stack.back() -= state[(int)instructionPointer.x][(int)instructionPointer.y-1][(int)instructionPointer.z];
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'S': // store(push) value of the cell above the current cell to stack(does not remove the value[citation needed])
            if (instructionPointer.y+1 < stateSize.y) {
				stack.push_back(state[(int)instructionPointer.x][(int)instructionPointer.y+1][(int)instructionPointer.z]);
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 's': // store(push) value of the cell below the current cell to stack
            if (instructionPointer.y-1 >= 0) {
				stack.push_back(state[(int)instructionPointer.x][(int)instructionPointer.y-1][(int)instructionPointer.z]);
            }
            moveInstructionPointerForward(instructionPointer, instructionDirection, stateSize);
            break;
		case 'X': // if the value on top of the stack is negative, turn the execution direction 90 degrees to the left
            if (stack.back() < 0) {
                bool clockWise = false;
                rotateInstructionDirection(clockWise, instructionDirection);
            }
            break;
		case 'x': // if the value on top of the stack is positive, turn the execution direction 90 degrees to the right
            if (stack.back() > 0) {
                bool clockWise = true;
                rotateInstructionDirection(clockWise, instructionDirection);
            }
            break;
		case '#': // # behaves just like 'j', but only if its position in the code / data array is 0, 0 (the left / top corner) and only if there is a '!' in the cell on its right side.
            break;

        default:
            // Raise an error if we are trying to execute a nonexistant instruction.
            break;
    }
    
}
