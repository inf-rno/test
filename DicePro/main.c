/*
 * Copyright (c) 2011-2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bbutil.h"

#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/sensor.h>
#include <bps/bps.h>
#include <bps/event.h>

#include <screen/screen.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

/**
 * Microseconds between accelerometer reads. This is the rate at which the
 * accelerometer data will be updated from hardware. The hardware update rate
 * is set below using sensor_set_rate.
 */
static const int ACCELEROMETER_RATE = 25000;


static GLfloat radio_btn_unselected_vertices[8], radio_btn_selected_vertices[8],
        background_portrait_vertices[8], background_landscape_vertices[8],
        *background_vertices;
static GLfloat radio_btn_unselected_tex_coord[8],
        radio_btn_selected_tex_coord[8], background_portrait_tex_coord[8],
        background_landscape_tex_coord[8], *background_tex_coord;
static GLuint radio_btn_unselected, radio_btn_selected, background_landscape,
        background_portrait, background, cube6[6];
static screen_context_t screen_cxt;
static font_t* font, *title_font, *subtitle_font;
static float width, height, anglex, angley, anglez;
static int shutdown, menu_active, menu_hide_animation, menu_show_animation;
static int selected;
static float cube_color[4];
static float menu_animation, menu_height, button_size_x, button_size_y, rollCount = 0;
static float pos_x, pos_y;
static float cube_pos_x, cube_pos_y, cube_pos_z;
static float posXi, posXs = 1,posYi, posYs = 1, posZi, posZs = 1;

GLfloat light_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat light_pos[] = { 0.0f, 25.0f, 0.0f, 1.0f };
GLfloat light_direction[] = { 0.0f, 0.0f, -30.0f, 1.0f };

static float cube_vertices[] = {
        // FRONT
        -2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f,
        2.0f,
        2.0f,
        2.0f,
        2.0f,
        2.0f,
        // BACK
        -2.0f, -2.0f, -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, -2.0f,
        -2.0f,
        2.0f,
        2.0f,
        -2.0f,
        // LEFT
        -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f, -2.0f, -2.0f,
        -2.0f,
        2.0f,
        -2.0f,
        // RIGHT
        2.0f, -2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f,
        2.0f,
        2.0f,
        // TOP
        -2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f,
        -2.0f,
        // BOTTOM
        -2.0f, -2.0f, 2.0f, -2.0f, -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f,
        -2.0f, };

float cube_normals[] = {
        // FRONT
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f,
        1.0f,
        // BACK
        0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f,
        -1.0f,
        // LEFT
        -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f,
        // RIGHT
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        // TOP
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        // BOTTOM
        0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        0.0f };
static const GLfloat cube6_tex_coord[] = {
       // Front face
	   0,0, 0.78,0, 0,0.78, 0.78,0.78,
       // Right face
	   0,0, 0.78,0, 0,0.78, 0.78,0.78,
       // Back face
	   0,0, 0.78,0, 0,0.78, 0.78,0.78,
       // Left face
	   0,0, 0.78,0, 0,0.78, 0.78,0.78,
       // Bottom face
	   0,0, 0.78,0, 0,0.78, 0.78,0.78,
       // Top face
	   0,0, 0.78,0, 0,0.78, 0.78,0.78
   };

static bool roll = false;

int resize();
void update();
void render();
int read_from_file();
void save_to_file();

void handleClick(int x, int y) {
    if (menu_active) {
        if ((y > menu_height - 4 * button_size_y)
                && (y < menu_height - 3 * button_size_y) && (x > 0)
                && (x < button_size_x)) {
            selected = 3;
            cube_color[0] = 1.0f;
            cube_color[1] = 0.0f;
            cube_color[2] = 0.0f;
            cube_color[3] = 1.0f;
        } else if ((y > menu_height - 3 * button_size_y)
                && (y < menu_height - 2 * button_size_y) && (x > 0)
                && (x < button_size_x)) {
            selected = 2;
            cube_color[0] = 0.0f;
            cube_color[1] = 1.0f;
            cube_color[2] = 0.0f;
            cube_color[3] = 1.0f;
        } else if ((y > menu_height - 2 * button_size_y)
                && (y < menu_height - button_size_y) && (x > 0)
                && (x < button_size_x)) {
            selected = 1;
            cube_color[0] = 0.0f;
            cube_color[1] = 0.0f;
            cube_color[2] = 1.0f;
            cube_color[3] = 1.0f;
        } else if ((y > menu_height - button_size_y) && (y < menu_height)
                && (x > 0) && (x < button_size_x)) {
            selected = 0;
            cube_color[0] = 1.0f;
            cube_color[1] = 1.0f;
            cube_color[2] = 0.0f;
            cube_color[3] = 1.0f;
        } else {
          if(!roll){roll = true;rollCount++;}
        }

        //Save current state to a file
        save_to_file();
    }
    else if(!roll){roll = true;rollCount++;}
}

static void handleScreenEvent(bps_event_t *event) {
    int screen_val, buttons;
    int pair[2];

    static bool mouse_pressed = false;

    screen_event_t screen_event = screen_event_get_event(event);

    //Query type of screen event and its location on the screen
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE,
            &screen_val);
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_SOURCE_POSITION,
            pair);

    //There is a difference between touch screen events and mouse events
    if (screen_val == SCREEN_EVENT_MTOUCH_RELEASE) {
        //Handle touch screen event
        handleClick(pair[0], pair[1]);

    } else if (screen_val == SCREEN_EVENT_POINTER) {
        //This is a mouse move event, it is applicable to a device with a usb mouse or simulator
        screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_BUTTONS,
                &buttons);

        if (buttons == SCREEN_LEFT_MOUSE_BUTTON) {
            //Left mouse button is pressed
            mouse_pressed = true;
        } else {
            if (mouse_pressed) {
                //Left mouse button was released, handle left click
                handleClick(pair[0], pair[1]);
                mouse_pressed = false;
            }
        }
    }
}

static void handleNavigatorEvent(bps_event_t *event) {
    switch (bps_event_get_code(event)) {
    case NAVIGATOR_ORIENTATION_CHECK:
        //Signal navigator that we intend to resize
        navigator_orientation_check_response(event, true);
        break;
    case NAVIGATOR_ORIENTATION:
        if (EXIT_FAILURE == resize(event)) {
            shutdown = true;
        }
        break;
    case NAVIGATOR_SWIPE_DOWN:
        if(menu_active)
        {
        	menu_hide_animation = true;
			menu_show_animation = false;
			menu_active = false;
        }
        else
		{
			menu_show_animation = true;
			menu_hide_animation = false;
		}

        break;
    case NAVIGATOR_EXIT:
        shutdown = true;
        break;
    case NAVIGATOR_WINDOW_INACTIVE:
    	sensor_stop_events(SENSOR_TYPE_ACCELEROMETER);
        //Wait for NAVIGATOR_WINDOW_ACTIVE event
        for (;;) {
            if (BPS_SUCCESS != bps_get_event(&event, -1)) {
                fprintf(stderr, "bps_get_event failed\n");
                break;
            }

            if (event && (bps_event_get_domain(event) == navigator_get_domain())) {
                int code = bps_event_get_code(event);
                if (code == NAVIGATOR_EXIT) {
                    shutdown = true;
                    break;
                } else if (code == NAVIGATOR_WINDOW_ACTIVE) {
                	sensor_request_events(SENSOR_TYPE_ACCELEROMETER);
                    break;
                }
            }
        }
        break;
    }
}

static void handle_events() {
	static float force_x, force_y, force_z;
    //Request and process available BPS events
    for(;;) {
        bps_event_t *event = NULL;
        if (BPS_SUCCESS != bps_get_event(&event, 0)) {
            fprintf(stderr, "bps_get_event failed\n");
            break;
        }

        if (event) {
            int domain = bps_event_get_domain(event);

            if (domain == screen_get_domain()) {
                handleScreenEvent(event);
            } else if (domain == navigator_get_domain()) {
                handleNavigatorEvent(event);
            } else if (domain == sensor_get_domain()) {
                if (!roll && SENSOR_ACCELEROMETER_READING == bps_event_get_code(event)) {
                    /*
                     * Extract the accelerometer data from the event and
                     * display it.
                     */
                    sensor_event_get_xyz(event, &force_x, &force_y, &force_z);
                    if(force_x > 5 || force_x < -5 || force_y > 7 || force_y < -7){roll = true;rollCount++;}

                    if(roll)
                    {
                    	posXs = force_x < 0 ? 1 : -1;
                    	posYs = force_y < 0 ? 1 : -1;
                    	posXi = fabs(force_x)*0.1;
                    	posYi = fabs(force_y)*0.1;
                    }

                }
            }
        } else {
            break;
        }
    }
}

int resize(bps_event_t *event) {
    //Query width and height of the window surface created by utility code
    EGLint surface_width, surface_height;

    if (event) {
        int angle = navigator_event_get_orientation_angle(event);

        //Let bbutil rotate current screen surface to this angle
        if (EXIT_FAILURE == bbutil_rotate_screen_surface(angle)) {
            fprintf(stderr, "Unable to handle orientation change\n");
            return EXIT_FAILURE;
        }
    }

    eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);

    EGLint err = eglGetError();
    if (err != 0x3000) {
        fprintf(stderr, "Unable to query EGL surface dimensions\n");
        return EXIT_FAILURE;
    }

    width = (float) surface_width;
    height = (float) surface_height;

    if (width > height) {
        cube_pos_x = 2.9f;
        cube_pos_y = 0.3f;
        cube_pos_z = -20.0f;

        background = background_landscape;
        background_vertices = background_landscape_vertices;
        background_tex_coord = background_landscape_tex_coord;

    } else {
        cube_pos_x = 0.0f;
        cube_pos_y = 0.0f;
        cube_pos_z = -30.0f;

        background = background_portrait;
        background_vertices = background_portrait_vertices;
        background_tex_coord = background_portrait_tex_coord;
    }

    update();

    if (event) {
        render();

        navigator_done_orientation(event);
    }

    return EXIT_SUCCESS;
}

int initialize() {
    EGLint surface_width, surface_height;

    //Load background and button textures
    float tex_x = 1.0f, tex_y = 1.0f;

    //Load textures for radio buttons
    int size_x = 64, size_y = 64;

    if (EXIT_SUCCESS
            != bbutil_load_texture("app/native/assets/radio_btn_unselected.png",
                    NULL, NULL, &tex_x, &tex_y, &radio_btn_unselected)) {
        fprintf(stderr, "Unable to load non-selected radio button texture\n");
    }

    radio_btn_unselected_vertices[0] = 0.0f;
    radio_btn_unselected_vertices[1] = 0.0f;
    radio_btn_unselected_vertices[2] = size_x;
    radio_btn_unselected_vertices[3] = 0.0f;
    radio_btn_unselected_vertices[4] = 0.0f;
    radio_btn_unselected_vertices[5] = size_y;
    radio_btn_unselected_vertices[6] = size_x;
    radio_btn_unselected_vertices[7] = size_y;

    radio_btn_unselected_tex_coord[0] = 0.0f;
    radio_btn_unselected_tex_coord[1] = 0.0f;
    radio_btn_unselected_tex_coord[2] = tex_x;
    radio_btn_unselected_tex_coord[3] = 0.0f;
    radio_btn_unselected_tex_coord[4] = 0.0f;
    radio_btn_unselected_tex_coord[5] = tex_y;
    radio_btn_unselected_tex_coord[6] = tex_x;
    radio_btn_unselected_tex_coord[7] = tex_y;

    if (EXIT_SUCCESS
            != bbutil_load_texture("app/native/assets/radio_btn_selected.png", NULL,
                    NULL, &tex_x, &tex_y, &radio_btn_selected)) {
        fprintf(stderr, "Unable to load selected radio button texture\n");
    }

    radio_btn_selected_vertices[0] = 0.0f;
    radio_btn_selected_vertices[1] = 0.0f;
    radio_btn_selected_vertices[2] = size_x;
    radio_btn_selected_vertices[3] = 0.0f;
    radio_btn_selected_vertices[4] = 0.0f;
    radio_btn_selected_vertices[5] = size_y;
    radio_btn_selected_vertices[6] = size_x;
    radio_btn_selected_vertices[7] = size_y;

    radio_btn_selected_tex_coord[0] = 0.0f;
    radio_btn_selected_tex_coord[1] = 0.0f;
    radio_btn_selected_tex_coord[2] = tex_x;
    radio_btn_selected_tex_coord[3] = 0.0f;
    radio_btn_selected_tex_coord[4] = 0.0f;
    radio_btn_selected_tex_coord[5] = tex_y;
    radio_btn_selected_tex_coord[6] = tex_x;
    radio_btn_selected_tex_coord[7] = tex_y;

    button_size_x = (float) size_x + 150;
    button_size_y = (float) size_y;

    size_x = 52;
    size_y = 54;

    if (EXIT_SUCCESS
   			!= bbutil_load_texture("app/native/assets/die6_1.png",
   					NULL, NULL, &tex_x, &tex_y, &cube6[0])) {
   		fprintf(stderr, "Unable to load non-selected die6_1 texture\n");
   	}

    if (EXIT_SUCCESS
       			!= bbutil_load_texture("app/native/assets/die6_2.png",
       					NULL, NULL, &tex_x, &tex_y, &cube6[1])) {
       		fprintf(stderr, "Unable to load non-selected die6_1 texture\n");
       	}

    if (EXIT_SUCCESS
			!= bbutil_load_texture("app/native/assets/die6_3.png",
					NULL, NULL, &tex_x, &tex_y, &cube6[2])) {
		fprintf(stderr, "Unable to load non-selected die6_3 texture\n");
	}
    if (EXIT_SUCCESS
       			!= bbutil_load_texture("app/native/assets/die6_4.png",
       					NULL, NULL, &tex_x, &tex_y, &cube6[3])) {
       		fprintf(stderr, "Unable to load non-selected die6_1 texture\n");
       	}

    if (EXIT_SUCCESS
       			!= bbutil_load_texture("app/native/assets/die6_5.png",
       					NULL, NULL, &tex_x, &tex_y, &cube6[4])) {
       		fprintf(stderr, "Unable to load non-selected die6_1 texture\n");
       	}

    if (EXIT_SUCCESS
			!= bbutil_load_texture("app/native/assets/die6_6.png",
					NULL, NULL, &tex_x, &tex_y, &cube6[5])) {
		fprintf(stderr, "Unable to load non-selected die6_6 texture\n");
	}

    eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);

    EGLint err = eglGetError();
    if (err != 0x3000) {
        fprintf(stderr, "Unable to query EGL surface dimensions\n");
        return EXIT_FAILURE;
    }

    width = (float) surface_width;
    height = (float) surface_height;

    if (EXIT_SUCCESS
            != bbutil_load_texture("app/native/assets/background-landscape.png", NULL, NULL,
                    &tex_x, &tex_y, &background_landscape)) {
        fprintf(stderr, "Unable to load landscape background texture\n");
    }

    size_x = (width > height) ? width : height;
    size_y = (width > height) ? height : width;

    background_landscape_vertices[0] = 0.0f;
    background_landscape_vertices[1] = 0.0f;
    background_landscape_vertices[2] = size_x;
    background_landscape_vertices[3] = 0.0f;
    background_landscape_vertices[4] = 0.0f;
    background_landscape_vertices[5] = size_y;
    background_landscape_vertices[6] = size_x;
    background_landscape_vertices[7] = size_y;

    background_landscape_tex_coord[0] = 0.0f;
    background_landscape_tex_coord[1] = 0.0f;
    background_landscape_tex_coord[2] = tex_x;
    background_landscape_tex_coord[3] = 0.0f;
    background_landscape_tex_coord[4] = 0.0f;
    background_landscape_tex_coord[5] = tex_y;
    background_landscape_tex_coord[6] = tex_x;
    background_landscape_tex_coord[7] = tex_y;

    if (EXIT_SUCCESS
            != bbutil_load_texture("app/native/assets/imgres1.png", NULL, NULL,
                    &tex_x, &tex_y, &background_portrait)) {
        fprintf(stderr, "Unable to load portrait background texture\n");
    }

    size_x = (height > width) ? width : height;
    size_y = (height > width) ? height : width;

    background_portrait_vertices[0] = 0.0f;
    background_portrait_vertices[1] = 0.0f;
    background_portrait_vertices[2] = size_x;
    background_portrait_vertices[3] = 0.0f;
    background_portrait_vertices[4] = 0.0f;
    background_portrait_vertices[5] = size_y;
    background_portrait_vertices[6] = size_x;
    background_portrait_vertices[7] = size_y;

    background_portrait_tex_coord[0] = 0.0f;
    background_portrait_tex_coord[1] = 0.0f;
    background_portrait_tex_coord[2] = tex_x;
    background_portrait_tex_coord[3] = 0.0f;
    background_portrait_tex_coord[4] = 0.0f;
    background_portrait_tex_coord[5] = tex_y;
    background_portrait_tex_coord[6] = tex_x;
    background_portrait_tex_coord[7] = tex_y;

    anglex = angley = anglez = 0.0f;
    pos_x = 0.0f;
    pos_y = 0.0f;

    //Load a typical arial font to use for our color menu
    int dpi = bbutil_calculate_dpi(screen_cxt);

    if (dpi == EXIT_FAILURE) {
        fprintf(stderr, "Unable to calculate dpi\n");
        return EXIT_FAILURE;
    }

    //As bbutil renders text using device-specifc dpi, we need to compute a point size
    //for the font, so that the text string fits into the bubble. Note that Playbook is used
    //as a reference point in this equation as we know that at dpi of 170, font with point size of
    //15 fits into the bubble texture.

    int point_size = (int)(15.0f / ((float)dpi / 170.0f ));

    font = bbutil_load_font("/usr/fonts/font_repository/monotype/arial.ttf", point_size, dpi);

    if (!font) {
       return EXIT_FAILURE;
    }

	point_size = (int)(35.0f / ((float)dpi / 170.0f ));

	title_font = bbutil_load_font("/usr/fonts/font_repository/monotype/arial.ttf", point_size, dpi);

	if (!title_font) {
	   return EXIT_FAILURE;
	}

	point_size = (int)(25.0f / ((float)dpi / 170.0f ));

	subtitle_font = bbutil_load_font("/usr/fonts/font_repository/monotype/arial.ttf", point_size, dpi);

	if (!subtitle_font) {
	   return EXIT_FAILURE;
	}

    float text_width, text_height;
    bbutil_measure_text(font, "Color Menu", &text_width, &text_height);
    menu_height = text_height + 10.0f + button_size_y * 4;

    //See if a savefile exists. If not, initialize to a hidden menu and a red cube.
    if (!read_from_file()) {
        selected = 3;
        cube_color[0] = 1.0f;
        cube_color[1] = 0.0f;
        cube_color[2] = 0.0f;
        cube_color[3] = 1.0f;

        menu_animation = 0.0f;
        menu_active = false;
        menu_show_animation = false;
        menu_hide_animation = false;
    }

    //Initialize positions of graphics assets on the screen, but don't resize the surface
    if (EXIT_FAILURE == resize(NULL)) {
        fprintf(stderr, "Initialize surface\n");
        return EXIT_FAILURE;
    }

    //Common gl setup
    glShadeModel(GL_SMOOTH);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);

    glEnable(GL_CULL_FACE);

    menu_show_animation = true;

    return EXIT_SUCCESS;
}

void enable_2d() {
    glViewport(0, 0, (int) width, (int) height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrthof(0.0f, width / height, 0.0f, 1.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1.0f / height, 1.0f / height, 1.0f);
}

void enable_3d() {
    glViewport(0, 0, (int) width, (int) height);

    GLfloat aspect_ratio = width / height;

    GLfloat fovy = 45.0f;
    GLfloat zNear = 1.0f;
    GLfloat zFar = 1000.0f;

    GLfloat top = tan(fovy * 0.0087266462599716478846184538424431f) * zNear;
    GLfloat bottom = -top;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustumf(aspect_ratio * bottom, aspect_ratio * top, bottom, top, zNear,
            zFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void update() {
	static float modx = 0, mody = 0, modz = 0;
	static float fac = 1,spin = 1, *angle1 = &anglex, *angle2 = &angley, *angle3 = &angley;
	static bool up = true, bounce = false;
	static float settleInc = 5;

	if(roll)
	{
		settleInc = 5;
		if(up && fac >50)up = false;
		else if(!up && fac<settleInc){roll = false;return;}
		fac = fac + (up?1:-0.3);//0.001
		spin = fac;//+0*max(up ? (float)(pow(2, fac)) : (float)(log(fac)*0.2), 1);
		(*angle1) = fmod(((*angle1) + spin), 360.0 );
		(*angle2) = fmod(((*angle2) + spin), 360.0 );
		(*angle3) = fmod(((*angle3) + spin), 360.0 );

	    if(posXi > 0.01 && cube_pos_x < -5){posXs = 1;bounce=true;}
	    if(posXi > 0.01 && cube_pos_x > 5){posXs = -1;bounce=true;}

	    if(posYi > 0.01 && cube_pos_y < -10){posYs = 1;bounce=true;}
	    if(posYi > 0.01 && cube_pos_y > 10){posYs = -1;bounce=true;}

	    if(posZi > 0.01 && cube_pos_z > -25){posZs = -1;}
	    if(posZi > 0.01 && cube_pos_z < -30){posZs = 1;bounce=true;}

	    if(cube_pos_z > -30 && posZi < 0.1){posZi = 0.2;posZs = -1;}
	    if(bounce)
	    {
	    	if(posXi > 0.1)	posXi = fabs(posXi - 0.1-posXi*1.6*(rand()%2));
	    	if(posYi > 0.1)	posYi = fabs(posYi - 0.1-posYi*1.6*(rand()%2));
	    	if(posZs == 1 && posZi > 0.1)	posZi = max(posZi - 0.2*(rand()%2), 0.0);

	    	switch(rand()%7)
	    	{
	    		case 0: angle1 = &anglex;angle2 = &angley;angle3 = &angley;break;
	    		case 1: angle1 = &anglex;angle2 = &anglez;angle3 = &anglez;break;
	    		case 2: angle1 = &angley;angle2 = &anglez;angle3 = &anglez;break;
	    		case 3: angle1 = &anglex;angle2 = &anglex;angle3 = &anglex;break;
	    		case 4: angle1 = &angley;angle2 = &angley;angle3 = &angley;break;
	    		case 5: angle1 = &anglez;angle2 = &anglez;angle3 = &anglez;break;
	    		case 6: angle1 = &anglex;angle2 = &angley;angle3 = &anglez;break;
	    		default:;
	    	}

	    	bounce = false;
	    }

	    cube_pos_x+=posXi * posXs;
	    cube_pos_y+=posYi * posYs;
	    cube_pos_z+=posZi * posZs;
	}
	else
	{
		up = true;fac = 1;spin = 1;bounce = false;posXi = posYi = posZi = 1;
		modx = fmod(anglex, 90-settleInc);
		mody = fmod(angley, 90-settleInc);
		modz = fmod(anglez, 90-settleInc);
		if(cube_pos_z>-30)cube_pos_z-=0.3;

		if(settleInc > 1 && (modx < 0.8*settleInc || mody < 0.8*settleInc || modz < 0.8*settleInc)) settleInc --;

		if(modx>45 && modx<90-settleInc)
		{modx+= settleInc;anglex+=settleInc;}
		if(modx<=45 && modx>0+settleInc)
		{modx-= settleInc;anglex-=settleInc;}

		if(mody>45 && mody<90-settleInc)
		{mody+= settleInc;angley+=settleInc;}
		if(mody<=45 && mody>0+settleInc)
		{mody-= settleInc;angley-=settleInc;}

		if(modz>45 && modz<90-settleInc)
		{modz+= settleInc;anglez+=settleInc;}
		if(modz<=45 && modz>0+settleInc)
		{modz-= settleInc;anglez-=settleInc;}
	}

	if(rollCount > 1){
		rollCount = 0;
		menu_hide_animation = true;
		menu_show_animation = false;
		menu_active = false;
	}

    if (menu_show_animation) {
        if (menu_animation < menu_height) {
            menu_animation += 7.0f;
        } else {
            menu_show_animation = false;
            menu_active = true;

            //Save current state to a file
            save_to_file();
        }
    } else if (menu_hide_animation) {
        if (menu_animation > 0.0f) {
            menu_animation -= 7.0f;
        } else {
            menu_hide_animation = false;
        }
    }

    pos_y = height - menu_animation;
}

void render() {
    int i;

    //Typical render pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //First render background and menu if it is enabled
    enable_2d();

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glVertexPointer(2, GL_FLOAT, 0, background_vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, background_tex_coord);
    glBindTexture(GL_TEXTURE_2D, background);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (menu_active || menu_show_animation || menu_hide_animation) {
        glTranslatef(pos_x, pos_y, 0.0f);

        for (i = 0; i < 4; i++) {
            if (i == selected) {
                glVertexPointer(2, GL_FLOAT, 0, radio_btn_selected_vertices);
                glTexCoordPointer(2, GL_FLOAT, 0, radio_btn_selected_tex_coord);
                glBindTexture(GL_TEXTURE_2D, radio_btn_selected);
            } else {
                glVertexPointer(2, GL_FLOAT, 0, radio_btn_unselected_vertices);
                glTexCoordPointer(2, GL_FLOAT, 0,
                        radio_btn_unselected_tex_coord);
                glBindTexture(GL_TEXTURE_2D, radio_btn_unselected);
            }

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glTranslatef(0.0f, 60.0f, 0.0f);
        }

        bbutil_render_text(font, "Color Menu", 10.0f, 10.0f, 0.35f, 0.35f, 0.35f, 1.0f);
        bbutil_render_text(font, "Red", 70.0f, -40.0f, 0.35f, 0.35f, 0.35f, 1.0f);
        bbutil_render_text(font, "Green", 70.0f, -100.0f, 0.35f, 0.35f, 0.35f, 1.0f);
        bbutil_render_text(font, "Blue", 70.0f, -160.0f, 0.35f, 0.35f, 0.35f, 1.0f);
        bbutil_render_text(font, "Yellow", 70.0f, -220.0f, 0.35f, 0.35f, 0.35f, 1.0f);
    }
    bbutil_render_text(title_font, "Shake or Tap", 150.0f, -350.0f, 0.35f, 0.35f, 0.35f, 1.0f);
    bbutil_render_text(subtitle_font, "Swipe Down for menu", 120.0f, -430.0f, 0.35f, 0.35f, 0.35f, 1.0f);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);

    //Then render the cube
    enable_3d();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    bbutil_render_text(subtitle_font, "Swipe Down for menu", 120.0f, -400.0f, 0.35f, 0.35f, 0.35f, 1.0f);
    glTranslatef(cube_pos_x, cube_pos_y, cube_pos_z);

    glRotatef(anglex, 1.0f, 0.0f, 0.0f);
    glRotatef(anglez, 0.0f, 0.0f, 1.0f);
    glRotatef(angley, 0.0f, 1.0f, 0.0f);

    glColor4f(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);

    glVertexPointer(3, GL_FLOAT, 0, cube_vertices);
    glNormalPointer(GL_FLOAT, 0, cube_normals);
    glTexCoordPointer(2, GL_FLOAT, 0, cube6_tex_coord);


    glBindTexture(GL_TEXTURE_2D, cube6[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, cube6[5]);
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
    glBindTexture(GL_TEXTURE_2D, cube6[3]);
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
    glBindTexture(GL_TEXTURE_2D, cube6[2]);
    glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
    glBindTexture(GL_TEXTURE_2D, cube6[4]);
    glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
    glBindTexture(GL_TEXTURE_2D, cube6[1]);
    glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    //Use utility code to update the screen
    bbutil_swap();
}

int read_from_file() {
    //open file as binary
    FILE *fp = fopen("data/save.dat", "rb");

    if (!fp) {
        return false;
    }

    int rc = fscanf(fp, "%i %i", &selected, &menu_active);

    if (rc == -1) {
        return false;
    } else {
        if (selected == 0) {
            cube_color[0] = 1.0f;
            cube_color[1] = 1.0f;
            cube_color[2] = 0.0f;
            cube_color[3] = 1.0f;
        } else if (selected == 1) {
            cube_color[0] = 0.0f;
            cube_color[1] = 0.0f;
            cube_color[2] = 1.0f;
            cube_color[3] = 1.0f;
        } else if (selected == 2) {
            cube_color[0] = 0.0f;
            cube_color[1] = 1.0f;
            cube_color[2] = 0.0f;
            cube_color[3] = 1.0f;
        } else if (selected == 3) {
            cube_color[0] = 1.0f;
            cube_color[1] = 0.0f;
            cube_color[2] = 0.0f;
            cube_color[3] = 1.0f;
        } else {
            return false;
        }
    }

    if (menu_active) {
        menu_animation = menu_height;
    }

    fclose(fp);
    return true;
}

void save_to_file() {
    //open file as binary
    FILE *fp = fopen("data/save.dat", "wb");

    if (!fp) {
        return;
    }

    fprintf(fp, "%i %i", selected, menu_active);

    fclose(fp);
}

int main(int argc, char *argv[]) {
    //Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context(&screen_cxt, 0);

    //Initialize BPS library
    bps_initialize();

    //Use utility code to initialize EGL for rendering with GL ES 1.1
    if (EXIT_SUCCESS != bbutil_init_egl(screen_cxt)) {
        fprintf(stderr, "bbutil_init_egl failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    //Initialize application logic
    if (EXIT_SUCCESS != initialize()) {
        fprintf(stderr, "initialize failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    //Signal BPS library that navigator and screen events will be requested
    if (BPS_SUCCESS != screen_request_events(screen_cxt)) {
        fprintf(stderr, "screen_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    if (BPS_SUCCESS != navigator_request_events(0)) {
        fprintf(stderr, "navigator_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }
    /*
	 * Before initializing the accelerometer service we must ensure the device
	 * supports it.
	 */
	if (!sensor_is_supported(SENSOR_TYPE_ACCELEROMETER)) {
		 fprintf(stderr, "Accelerometer not supported by device!");
		bbutil_terminate();
		screen_destroy_context(screen_cxt);
		bps_shutdown();
		return 0;
	}

    /*
         * Initialize the accelerometer by setting the rates at which the
         * accelerometer values will be updated from hardware
         */
        sensor_set_rate(SENSOR_TYPE_ACCELEROMETER, ACCELEROMETER_RATE);
        sensor_set_skip_duplicates(SENSOR_TYPE_ACCELEROMETER, true);
        sensor_request_events(SENSOR_TYPE_ACCELEROMETER);

    while (!shutdown) {
        // Handle user input and accelerometer
        handle_events();
        // Update scene contents
        update();
        // Draw Scene
        render();
    }

    //Stop requesting events from libscreen
    screen_stop_events(screen_cxt);

    //Use utility code to terminate EGL setup
    bbutil_terminate();

    //Shut down BPS library for this process
    bps_shutdown();

    //Destroy libscreen context
    screen_destroy_context(screen_cxt);
    return 0;
}
