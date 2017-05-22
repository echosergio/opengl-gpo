CC = g++
CC_FLAGS = -w
LD_FLAGS = -L../glsdk/freeglut/lib -lfreeglutD -L../glsdk/glfw/library -lglfwD -L../glsdk/glutil/lib -lglutilD -L../glsdk/glload/lib -lglloadD -L../glsdk/glimg/lib -lglimgD -L../glsdk/glmesh/lib -lglmeshD -lGL -lGLU -lm -lX11 -ldl -lXext -lz
INCLUDE_FLAGS = -I ../glsdk/glfw/include/ -I ../glsdk/glload/include/  -I ../glsdk/glm/

EXEC = gpo.app
SOURCES = $(wildcard ./*.cpp) $(wildcard ../cross-cutting/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LD_FLAGS)

%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $(INCLUDE_FLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)

