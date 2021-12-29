#include "physics.h"
#include "graphics.h"
#include "menus.h"
#include "main.h"

int exampleNumberCapture = 15;
std::string exampleTextCapture = std::string("exampleText");


unsigned long int ticks = 0;


class Branch: public PhysicalObject
{
public:
	float energyValue;

	b2Color color;
	float rootThickness;
	float tipThickness;
	float length;
	float naturalAngle;

	std::list<Branch> branches;

	bool ready;

	unsigned int capturedLight;

	Branch();
};


struct Tree
{

	float energyStored;

	std::string genes;

	unsigned long int lastReproduced;

	bool ready;


	std::list<Branch> branches;


	uint geneCursor;
	float thicknessCursor;
	float angleCursor;
	b2Color colorCursor;

	Tree();


}

Branch::Branch(float rootThickness, float tipThickness, float length, float angle, b2Color color)
{

	this-> energyValue = ((rootThickness + tipThickness) / 2) * length;

	this->color = color;
	this->rootThickness = rootThickness;
	this->tipThickness  = tipThickness;
	this->length        = length;
	this->naturalAngle  = angle;

	this-> branches = std::list<Branch>();

	this-> ready = true;

	this->capturedLight = 0;

}

Tree::Tree(std::string genes)
{


	this-> energyStored = 0;

	this-> genes = genes;

	this-> lastReproduced = ticks;

	this-> ready = true;


	this->  branches = std::list<Branch>();


	this-> geneCursor = 0;
	this-> thicknessCursor = 0.1f;
	this-> angleCursor = 0.0f;
	this->colorCursor = b2Color(0.0f, 0.0f, 0.0f);



}

// return 1 indicates the sequence should break. return 0 means it should continue.
int grow( Tree * tree , Branch * growingBranch)
{

	if (tree->energyStored > 0)
	{



		switch (tree->genes[geneCursor])
		{

		case 'a':
		{
			// array n times end-to-end.
			geneCursor++;
			int arrayN = alphanumeric( tree->genes[geneCursor] );

			float tempThicknessCursor = tree->thicknessCursor;
			float tempLengthCursor    = tree->lengthCursor;
			float tempAngleCursor     = tree->angleCursor;
			b2Color tempColorCursor   = tree->colorCursor;

			for (int i = 0; i < arrayN; ++i)
			{
				while (!grow()) { ; }
			}

			tree->thicknessCursor = tempThicknessCursor;
			tree->lengthCursor = tempLengthCursor;
			tree->angleCursor = tempAngleCursor;
			tree->colorCursor = tempColorCursor;


			break;
		}
		case 'b':
		{
			// array n times around the parent.

			// array n times end-to-end.
			geneCursor++;
			int arrayN = alphanumeric( tree->genes[geneCursor] );

			float tempThicknessCursor = tree->thicknessCursor;
			float tempLengthCursor    = tree->lengthCursor;
			float tempAngleCursor     = tree->angleCursor;
			b2Color tempColorCursor   = tree->colorCursor;

			for (int i = 0; i < arrayN; ++i)
			{
				while (!grow()) { ; }
				tree->thicknessCursor = tempThicknessCursor;
				tree->lengthCursor = tempLengthCursor;
				tree->angleCursor = tempAngleCursor;
				tree->colorCursor = tempColorCursor;
			}


			break;
		}
		case 'c':
		{
			// fractal: array n times end-to-end with accumulating scale.
			geneCursor++;
			int arrayN = alphanumeric( tree->genes[geneCursor] );

			geneCursor++;
			float fractalScale = ( alphanumeric( tree->genes[geneCursor] ) ) / 13;

			float tempThicknessCursor = tree->thicknessCursor;
			float tempLengthCursor    = tree->lengthCursor;
			float tempAngleCursor     = tree->angleCursor;
			b2Color tempColorCursor   = tree->colorCursor;

			for (int i = 0; i < arrayN; ++i)
			{
				while (!grow()) { ; }

				tree->thicknessCursor = tree->thicknessCursor * fractalScale;
				tree->lengthCursor    = tree->lengthCursor * fractalScale;
			}

			tree->thicknessCursor = tempThicknessCursor;
			tree->lengthCursor = tempLengthCursor;
			tree->angleCursor = tempAngleCursor;
			tree->colorCursor = tempColorCursor;

			break;
		}
		case 'd':
		{
			// draw a branch.



			b2Vec2 vertices[] = {
				b2Vec2( + (tree->thicknessCursor / 2),  -(tree->lengthCursor / 2)),
				b2Vec2(- (tree->thicknessCursor / 2),  -(tree->lengthCursor / 2)),
				b2Vec2( - (tree->thicknessCursor / 2),  +(tree->lengthCursor / 2)),
				b2Vec2(+ (tree->thicknessCursor / 2),  +(tree->lengthCursor / 2))
			};



			growingBranch.push_back( Branch( tree->thicknessCursor, tree->thicknessCursor, tree->lengthCursor, tree->angleCursor, tree->colorCursor) );

			addToWorld(tree->branches.back());

			break;
		}
		case 'e':
		{
			// break array or sequence
			return 1;
			break;
		}
		case 'f':
		{
			// add cumulative angle

			geneCursor++;
			tree->angleCursor += alphanumeric( tree->genes[geneCursor] );
			break;

		}
		case 'g':
		{
			// reset cumulative angle to

			geneCursor++;
			tree->angleCursor = alphanumeric( tree->genes[geneCursor] );
			break;

		}

		case 'h':
		{
			// branch- do a single sequence at an angle.
			break;

		}
		case 'i':
		{
			// reset length cursor
			geneCursor++;
			tree->lengthCursor = alphanumeric(tree->genes[geneCursor] );
			break;

		}
		case 'j':
		{
			// reset thickness cursor
			geneCursor++;
			tree->thicknessCursor = alphanumeric(tree->genes[geneCursor] );
			break;

		}

		case 'k':
		{
			// add red component
			geneCursor++;
			// tree->thicknessCursor = alphanumeric(tree->genes[geneCursor] );
			break;

		}



		case 'l':
		{
			// add green component
			geneCursor++;
			// tree->thicknessCursor = alphanumeric(tree->genes[geneCursor] );
			break;

		}




		case 'm':
		{
			// add blue component
			geneCursor++;
			// tree->thicknessCursor = alphanumeric(tree->genes[geneCursor] );
			break;

		}









		}
		geneCursor++;



	}

	return 0;


}


void sprout () {

}


std::list<Tree> garden;





void rebuildMenus ()
{
	int spacing = 10;

	menuItem * exampleMenuRoot = setupMenu ( std::string ("menu") , RIGHT, nullptr, (void *)exampleMenuCallback, nullptr, b2Color(0.1f, 0.1f, 0.1f, 1.0f), b2Vec2(500, 500));
	exampleMenuRoot->collapsed = false;

	uDataWrap *     tempDataWrap = new uDataWrap( (void*)&exampleNumberCapture, TYPE_UDATA_INT  );
	menuItem * exampleMenuNumber = setupMenu ( std::string ("editable number") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap);
	exampleMenuNumber->collapsed = false;


	menus.push_back(*exampleMenuRoot);
}

void initializeGame ()
{
	// float exampleBoxSize = 10.0f;
	// std::vector<b2Vec2> exampleBoxVertices =
	// {
	// 	b2Vec2( +1 * exampleBoxSize ,  -1 * exampleBoxSize), //b2Vec2 rootVertexA =
	// 	b2Vec2( -1 * exampleBoxSize ,  -1 * exampleBoxSize), // b2Vec2 rootVertexB =
	// 	b2Vec2( -1 * exampleBoxSize ,  +1 * exampleBoxSize), //b2Vec2 tipVertexA =
	// 	b2Vec2( +1 * exampleBoxSize ,  +1 * exampleBoxSize) // b2Vec2 tipVertexB =
	// };
	// addToWorld( PhysicalObject(exampleBoxVertices, false), b2Vec2(0.0f, 20.0f) , 0.0f);

	std::vector<b2Vec2> exampleBox2Vertices =
	{
		b2Vec2( +100 * exampleBoxSize ,  -1 * exampleBoxSize), //b2Vec2 rootVertexA =
		b2Vec2( -100 * exampleBoxSize ,  -1 * exampleBoxSize), // b2Vec2 rootVertexB =
		b2Vec2( -100 * exampleBoxSize ,  +1 * exampleBoxSize), //b2Vec2 tipVertexA =
		b2Vec2( +100 * exampleBoxSize ,  +1 * exampleBoxSize) // b2Vec2 tipVertexB =
	};
	addToWorld( PhysicalObject(exampleBox2Vertices, true) , b2Vec2(0.0f, -20.0f), 0.0f );
}

void threadGame()
{


	ticks++;
}

void gameGraphics()
{
	/** your graphics logic here. turn your data into floats and pack it into vertex_buffer_data. The sequence is r, g, b, a, x, y; repeat for each point. **/

	unsigned int nVertsToRenderThisTurn = 0;
	unsigned int nIndicesToUseThisTurn = 0;

	std::list<PhysicalObject>::iterator object;
	for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	{
		unsigned int nObjectVerts = object->vertices.size();
		nVertsToRenderThisTurn += nObjectVerts;
		nIndicesToUseThisTurn  += nObjectVerts + 1;
	}

	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
	unsigned int vertex_buffer_cursor = 0;
	float vertex_buffer_data[totalNumberOfFields];
	unsigned int index_buffer_cursor = 0;
	unsigned int index_buffer_content = 0;
	unsigned int index_buffer_data[nIndicesToUseThisTurn];

	// std::list<PhysicalObject>::iterator object;
	for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	{

		b2Vec2 bodyPosition = object->p_body->GetWorldCenter();
		float bodyAngle = object->p_body->GetAngle();
		float bodyAngleSin = sin(bodyAngle);
		float bodyAngleCos = cos(bodyAngle);

		std::vector<b2Vec2>::iterator vert;
		for (vert = std::begin(object->vertices); vert !=  std::end(object->vertices); ++vert)
		{
			// add the position and rotation of the game-world object that the vertex belongs to.
			b2Vec2 rotatedPoint = b2Vec2(   vert->x + bodyPosition.x, vert->y + bodyPosition.y   );
			rotatedPoint = b2RotatePointPrecomputed( bodyPosition, bodyAngleSin, bodyAngleCos, rotatedPoint);

			vertex_buffer_data[(vertex_buffer_cursor) + 0] = object->color.r;
			vertex_buffer_data[(vertex_buffer_cursor) + 1] = object->color.g;
			vertex_buffer_data[(vertex_buffer_cursor) + 2] = object->color.b;
			vertex_buffer_data[(vertex_buffer_cursor) + 3] = object->color.a;
			vertex_buffer_data[(vertex_buffer_cursor) + 4] = rotatedPoint.x;
			vertex_buffer_data[(vertex_buffer_cursor) + 5] = rotatedPoint.y ;
			(vertex_buffer_cursor) += 6;

			index_buffer_data[(index_buffer_cursor)] = (index_buffer_content);
			(index_buffer_cursor)++;
			(index_buffer_content)++;
		}

		index_buffer_data[(index_buffer_cursor)] = PRIMITIVE_RESTART;
		(index_buffer_cursor)++;
	}

	glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_buffer_data ), vertex_buffer_data, GL_DYNAMIC_DRAW );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_DYNAMIC_DRAW);
	glDrawElements( GL_TRIANGLE_FAN, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );
}