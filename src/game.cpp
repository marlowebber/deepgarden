#include "physics.h"
#include "graphics.h"
#include "menus.h"
#include "main.h"

int exampleNumberCapture = 15;
std::string exampleTextCapture = std::string("exampleText");


unsigned long int ticks = 0;

float thicknessCursor = 1.0f;
float lengthCursor = 1.0f;;
float angleCursor = 0.0f;;
b2Color colorCursor = b2Color(0.0f, 0.0f, 0.0f, 1.0f);

b2Vec2 worldPositionCursor = b2Vec2(0.0f, 0.0f);


struct Branch
{
	float energyValue;

	b2Color color;
	float naturalAngle;

	std::list<Branch> branches;

	bool ready;

	unsigned int capturedLight;

	std::vector<b2Vec2> vertices =//
	{
		b2Vec2( +1.0f ,  -1.0f), //b2Vec2 rootVertexA =
		b2Vec2( -1.0f ,  -1.0f), // b2Vec2 rootVertexB =
		b2Vec2( -1.0f ,  +1.0f), //b2Vec2 tipVertexA =
		b2Vec2( +1.0f ,  +1.0f) // b2Vec2 tipVertexB =
	};

	PhysicalObject object = PhysicalObject(  vertices, false);

	// Branch();

	Branch()
	{

		this-> energyValue = thicknessCursor * lengthCursor;

		this->color = color;
		this->naturalAngle  = angleCursor;

		this-> branches = std::list<Branch>();

		this-> ready = true;

		this->capturedLight = 0;

		this-> vertices = {
			b2Vec2( + (thicknessCursor / 2),  -(lengthCursor / 2)),
			b2Vec2( - (thicknessCursor / 2),  -(lengthCursor / 2)),
			b2Vec2( - (thicknessCursor / 2),  +(lengthCursor / 2)),
			b2Vec2( + (thicknessCursor / 2),  +(lengthCursor / 2))
		};


		this->object = PhysicalObject(this->vertices, false);

	}

};

Branch * lastTouchedBranch = nullptr;


struct Tree
{

	float energyStored;

	std::string genes;

	unsigned long int lastReproduced;

	bool ready;


	std::list<Branch> branches;


	uint geneCursor;

	// Tree();
	Tree(std::string genes)
	{


		this-> energyStored = 1;

		this-> genes = genes;

		printf("new tree: %s\n", genes.c_str());

		this-> lastReproduced = ticks;

		this-> ready = true;


		this->  branches = std::list<Branch>();


		this-> geneCursor = 0;



	}


};




// return 1 indicates the sequence should break. return 0 means it should continue.
int grow( Tree * tree , Branch * growingBranch)
{

	// printf()
	if (tree->geneCursor < tree->genes.length() )
	{



		switch (tree->genes[tree->geneCursor])
		{


		case '.':
		{
			printf("break sequence\n");
			return 1;
			break;
		}
		case ' ':
		{

			printf("break sequence\n");
			return 1;
			break;
		}
		case 'a':
		{
			// array n times end-to-end.

			// int arrayN = alphanumeric( tree->genes[tree->geneCursor] );

			// // the segment shape parameters are reset, but angle and position are allowed to accumulate.
			// float tempThicknessCursor = thicknessCursor;
			// float tempLengthCursor    = lengthCursor;
			// b2Color tempColorCursor   = colorCursor;

			// for (int i = 0; i < arrayN; ++i)
			// {
			// 	while (!grow( tree, lastTouchedBranch )) { ; }
			// }

			// thicknessCursor = tempThicknessCursor;
			// lengthCursor = tempLengthCursor;
			// colorCursor = tempColorCursor;


			break;
		}
		case 'b':
		{
			// array n times in an even circular radiation.

			// int arrayN = alphanumeric( tree->genes[tree->geneCursor] );

			// float radiateAngle = (2 * const_pi) / arrayN;

			// float tempThicknessCursor = thicknessCursor;
			// float tempLengthCursor    = lengthCursor;
			// float tempAngleCursor     = angleCursor;
			// b2Color tempColorCursor   = colorCursor;
			// b2Vec2 tempWorldPositionCursor = worldPositionCursor;

			// for (int i = 0; i < arrayN; ++i)
			// {
			// 	while (!grow( tree, growingBranch )) { ; }
			// 	thicknessCursor = tempThicknessCursor;
			// 	lengthCursor = tempLengthCursor;
			// 	angleCursor = tempAngleCursor + (i  * radiateAngle);
			// 	colorCursor = tempColorCursor;
			// 	worldPositionCursor  = tempWorldPositionCursor;

			// }


			// thicknessCursor = tempThicknessCursor;
			// lengthCursor = tempLengthCursor;
			// angleCursor = tempAngleCursor;
			// colorCursor = tempColorCursor;
			// worldPositionCursor  = tempWorldPositionCursor;



			break;
		}
		case 'c':
		{
			// fractal: array n times end-to-end with accumulating scale.

			// int arrayN = alphanumeric( tree->genes[tree->geneCursor] );

			// tree->geneCursor++;
			// float fractalScale = ( alphanumeric( tree->genes[tree->geneCursor] ) ) / 13;

			// float tempThicknessCursor = thicknessCursor;
			// float tempLengthCursor    = lengthCursor;

			// for (int i = 0; i < arrayN; ++i)
			// {
			// 	while (!grow(tree, lastTouchedBranch)) { ; }

			// 	thicknessCursor = thicknessCursor * fractalScale;
			// 	lengthCursor    = lengthCursor * fractalScale;
			// }

			// thicknessCursor = tempThicknessCursor;
			// lengthCursor = tempLengthCursor;

			break;
		}
		case 'd':
		{
			// draw a branch.


			// tree->geneCursor++;
			printf("draw a branch\n");


			growingBranch->branches.push_back( Branch() );


			b2Vec2 newBranchPosition = b2Vec2(

			                               worldPositionCursor.x + ((lengthCursor / 2) * cos(angleCursor))
			                               ,

			                               worldPositionCursor.y + ((lengthCursor / 2) * sin(angleCursor))
			                           );


			Branch * newBranch = &(growingBranch->branches.back());
			addToWorld( &(growingBranch->branches.back().object), newBranchPosition , angleCursor  );

			






			// the world cursor is moved from the root to the tip position

			worldPositionCursor = b2Vec2(
			                          worldPositionCursor.x + (lengthCursor * cos(angleCursor))
			                          ,

			                          worldPositionCursor.y + (lengthCursor * sin(angleCursor))
			                      );


			tree->energyStored -= lengthCursor * thicknessCursor;


			if (
			    growingBranch != nullptr &&
			    lastTouchedBranch != nullptr 
			)
			{
				if (
				    growingBranch != newBranch
				)
				{

					if (
					    growingBranch->object.p_body != nullptr &&
					    lastTouchedBranch->object.p_body != nullptr 
					)
					{

						printf("create joint\n");
						b2RevoluteJointDef jointDef =  b2RevoluteJointDef();
						jointDef.collideConnected = false; // this means that limb segments dont collide with their children
						jointDef.bodyA = lastTouchedBranch->object.p_body;
						jointDef.bodyB = newBranch->object.p_body;
						jointDef.localAnchorA = b2Vec2( 0.0f,   -1 * (lengthCursor / 2) );
						jointDef.localAnchorB = b2Vec2( 0.0f,   1 * (lengthCursor / 2) );

						m_world->CreateJoint( &(jointDef) );

					}
				}
			}

			lastTouchedBranch = newBranch;

			break;
		}
		case 'e':
		{
			// break array or sequence
			printf("break sequence\n");
			return 1;
			break;
		}
		case 'f':
		{
			// add cumulative angle
//
			// tree->geneCursor++;
			// angleCursor += alphanumeric( tree->genes[tree->geneCursor] );
			break;

		}
		case 'g':
		{
			// reset cumulative angle to

			// tree->geneCursor++;
			// angleCursor = alphanumeric( tree->genes[tree->geneCursor] );
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
			// tree->geneCursor++;
			// lengthCursor = alphanumeric(tree->genes[tree->geneCursor] );
			break;

		}
		case 'j':
		{
			// reset thickness cursor
			// tree->geneCursor++;
			// thicknessCursor = alphanumeric(tree->genes[tree->geneCursor] );
			break;

		}

			// case 'k':
			// {
			// 	// add red component
			// 	// tree->geneCursor++;
			// 	// tree->thicknessCursor = alphanumeric(tree->genes[geneCursor] );
			// 	break;

			// }



			// case 'l':
			// {
			// 	// add green component
			// 	// tree->geneCursor++;
			// 	// tree->thicknessCursor = alphanumeric(tree->genes[geneCursor] );
			// 	break;

			// }




			// case 'm':
			// {
			// 	// add blue component
			// 	tree->geneCursor++;
			// 	// tree->thicknessCursor = alphanumeric(tree->genes[geneCursor] );
			// 	break;

			// }









		}
		// tree->geneCursor++;
		tree->geneCursor++;

		if (tree->geneCursor > tree->genes.length() )
		{
			return 1;
		}



	}

	return 0;


}


// void sprout ()
// {

// }

std::list<Tree> garden;

void instantiateSeed (std::string genes)
{



	garden.push_back(  Tree(genes)  );

	Tree * tree = &(garden.back()) ;

	// tree->geneCursor++;
	// angleCursor = alphanumeric( tree->genes[tree->geneCursor] );

	// tree->geneCursor++;
	// lengthCursor = alphanumeric( tree->genes[tree->geneCursor] );

	// tree->geneCursor++;
	// thicknessCursor = alphanumeric( tree->genes[tree->geneCursor] );


	tree->branches.push_back( Branch() );


	b2Vec2 newBranchPosition = b2Vec2(

	                               worldPositionCursor.x + ((lengthCursor / 2) * cos(angleCursor))
	                               ,

	                               worldPositionCursor.y + ((lengthCursor / 2) * sin(angleCursor))
	                           );


	addToWorld( &(tree->branches.back().object), newBranchPosition , angleCursor  );

	lastTouchedBranch = &(tree->branches.back());


}








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
	float exampleBoxSize = 10.0f;
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
	addToWorld( new PhysicalObject(exampleBox2Vertices, true) , b2Vec2(0.0f, -20.0f), 0.0f );


	std::string exampleSentence = std::string("ddd");


	instantiateSeed(exampleSentence);


}

void threadGame()
{

	std::list<Tree>::iterator tree;
	for (tree = garden.begin(); tree != garden.end(); ++tree)
	{
		grow( &(*tree), &(tree->branches.back()));
	}


	ticks++;
}

void gameGraphics()
{
	/** your graphics logic here. turn your data into floats and pack it into vertex_buffer_data. The sequence is r, g, b, a, x, y; repeat for each point. **/

	unsigned int nVertsToRenderThisTurn = 0;
	unsigned int nIndicesToUseThisTurn = 0;

	std::list<PhysicalObject*>::iterator object;
	for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	{
		unsigned int nObjectVerts = (*object)->vertices.size();
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

		b2Vec2 bodyPosition = (*object)->p_body->GetWorldCenter();
		float bodyAngle = (*object)->p_body->GetAngle();
		float bodyAngleSin = sin(bodyAngle);
		float bodyAngleCos = cos(bodyAngle);

		std::vector<b2Vec2>::iterator vert;
		for (vert = std::begin((*object)->vertices); vert !=  std::end((*object)->vertices); ++vert)
		{
			// add the position and rotation of the game-world object that the vertex belongs to.
			b2Vec2 rotatedPoint = b2Vec2(   vert->x + bodyPosition.x, vert->y + bodyPosition.y   );
			rotatedPoint = b2RotatePointPrecomputed( bodyPosition, bodyAngleSin, bodyAngleCos, rotatedPoint);

			vertex_buffer_data[(vertex_buffer_cursor) + 0] = (*object)->color.r;
			vertex_buffer_data[(vertex_buffer_cursor) + 1] = (*object)->color.g;
			vertex_buffer_data[(vertex_buffer_cursor) + 2] = (*object)->color.b;
			vertex_buffer_data[(vertex_buffer_cursor) + 3] = (*object)->color.a;
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