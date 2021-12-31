#include "physics.h"
#include "graphics.h"
#include "menus.h"
#include "main.h"

int exampleNumberCapture = 15;
std::string exampleTextCapture = std::string("exampleText");

b2Vec2 wind = b2Vec2(0.0f, 0.0f);

unsigned long int ticks = 0;

float thicknessCursor  		= 1.0f;
float lengthCursor 			= 1.0f;;
float angleCursor 			= 0.0f;;
b2Color colorCursor 		= b2Color(0.0f, 0.0f, 0.0f, 1.0f);
b2Vec2 worldPositionCursor 	= b2Vec2(0.0f, 0.0f);

Branch::Branch()
{
	this->owner = nullptr;
	this-> energyValue = thicknessCursor * lengthCursor;
	this->color = color;
	this->naturalAngle  = angleCursor;
	this-> branches = std::list<Branch>();
	this->capturedLight = 0;
	this-> vertices =
	{
		b2Vec2( + (thicknessCursor / 2),  -(lengthCursor / 2)),
		b2Vec2( - (thicknessCursor / 2),  -(lengthCursor / 2)),
		b2Vec2( - (thicknessCursor / 2),  +(lengthCursor / 2)),
		b2Vec2( + (thicknessCursor / 2),  +(lengthCursor / 2))
	};
	this->seed = false;
	this->object = PhysicalObject(this->vertices, false);
	this->flagDelete = false;
	this->stem = false;
}

Branch * lastTouchedBranch = nullptr;

Tree::Tree(std::string genes)
{
	this->flagDelete = false;
	this-> energyStored = 1;
	this-> genes = genes;
	printf("new tree: %s\n", genes.c_str());
	this-> lastReproduced = ticks;
	this->mature = false;
	this->  branches = std::list<Branch>();
	this-> geneCursor = 0;
	this->germinated = false;
	this->sproutPosition = b2Vec2(0.0f, 0.0f);
	this->affixedObject = nullptr;

}

std::list<Tree> garden;

void instantiateSeed (std::string genes, Branch * joinBranch)
{
	garden.push_back(  Tree(genes)  );
	Tree * tree = &(garden.back()) ;
	tree->branches.push_back( Branch() );
	Branch * newBranch = &(tree->branches.back());
	newBranch->object.color = b2Color(0.8f, 0.8f, 0.1f, 1.0f);
	newBranch->seed = true;
	tree->mature = false;
	tree->germinated = false;
	b2Vec2 newBranchPosition ;

	if (joinBranch != nullptr)
	{
		newBranchPosition = b2Vec2(

		                        joinBranch ->object.p_body->GetWorldCenter() .x //+ ((1.0f ) * cos(joinBranch ->object.p_body->GetAngle()))
		                        ,


		                        joinBranch ->object.p_body->GetWorldCenter() .y //+ ((1.0f ) * sin(joinBranch ->object.p_body->GetAngle()))
		                    );
	}

	else {
		newBranchPosition = b2Vec2(

		                        worldPositionCursor.x + ((lengthCursor / 2) * cos(angleCursor))
		                        ,

		                        worldPositionCursor.y + ((lengthCursor / 2) * sin(angleCursor))
		                    );

	}

	addToWorld( &(newBranch->object), newBranchPosition , angleCursor  );
	newBranch->object.owner = newBranch;
	newBranch->owner = tree;
	lastTouchedBranch = newBranch;
	if (joinBranch != nullptr)
	{
		// createJoint( &(newBranch->object), &(joinBranch->object) );
	}
}

// return 1 indicates the sequence should break. return 0 means it should continue.
int grow( Tree * tree , Branch * growingBranch)
{

	if (tree->geneCursor >= tree->genes.length() ) {return 1;}

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
			tree->geneCursor++;

			uint arrayN = alphanumeric( tree->genes[tree->geneCursor] );
			printf("array %i times\n", arrayN);

			// the segment shape parameters are reset, but angle and position are allowed to accumulate.
			float tempThicknessCursor = thicknessCursor;
			float tempLengthCursor    = lengthCursor;
			b2Color tempColorCursor   = colorCursor;

			uint tempGeneCursor = tree->geneCursor;

			for (uint i = 0; i < arrayN; ++i)
			{

				tree->geneCursor = tempGeneCursor;
				bool proceed = true;
				while (proceed) {

					if ( grow( tree, lastTouchedBranch ) == 1 )
					{
						proceed = false;
					}

					if (tree->geneCursor >= tree->genes.length())
					{
						proceed = false;

					}
				}
			}

			thicknessCursor = tempThicknessCursor;
			lengthCursor = tempLengthCursor;
			colorCursor = tempColorCursor;


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
		case 's':
		{
			// draw a seed

			instantiateSeed(tree->genes, lastTouchedBranch);
			// return 1; // a seed or fruit is always the end of the branch

			break;
		}
		case 'd':
		{
			// draw a branch.

			printf("draw a branch\n");
			Branch * newBranch ;
			if (growingBranch->seed)
			{
				tree->branches.push_back( Branch() );
				newBranch = &(tree->branches.back());
				newBranch->seed = false;
				if (tree->mature && !tree->germinated)
				{
					tree->germinated = true;
				}
			}
			else
			{
				growingBranch->branches.push_back( Branch() );
				newBranch = &(growingBranch->branches.back());

			}

			b2Vec2 newBranchPosition = b2Vec2(
			                               worldPositionCursor.x + ((lengthCursor / 2) * cos(angleCursor)),
			                               worldPositionCursor.y + ((lengthCursor / 2) * sin(angleCursor))
			                           );

			newBranch->object.owner = newBranch;
			newBranch->owner = tree;
			addToWorld( &(newBranch->object), newBranchPosition , angleCursor  );

			// the world cursor is moved from the root to the tip position
			worldPositionCursor = b2Vec2(
			                          worldPositionCursor.x + (lengthCursor * cos(angleCursor)),
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
						if (  (!( growingBranch->seed ))  && (!( lastTouchedBranch->seed ))  )
						{
							createJoint( &(newBranch->object), &(lastTouchedBranch->object) );
						}

						if (growingBranch->seed)
						{

							if (tree->affixedObject != nullptr)
							{

								printf("a tree grew in the ground\n");

								b2Vec2 bLocalAnchor = b2Vec2(  tree->sproutPosition.x - tree->affixedObject->p_body->GetWorldCenter().x , tree->sproutPosition.y - tree->affixedObject->p_body->GetWorldCenter().y   );

								createJointWithVariableBAnchor( &(newBranch->object), tree->affixedObject , bLocalAnchor );

								newBranch->stem = true;
								// createJointW
							}


						}
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


	std::string exampleSentence = std::string("daffffd s");


	instantiateSeed(exampleSentence, nullptr);
}


void recursiveUpdateMotors( Branch * branch )
{


	if (branch->stem)
	{

		if (branch->object.p_joint != nullptr)
		{


			float motorSpeed = constrainAngle((branch->object.p_body->GetAngle() +(0.5*const_pi)))  * -0.1;
			// branch->object.p_joint->SetMotorSpeed(motorSpeed);


		}

	}



	printf("bramches:%lu\n", branch->branches.size());
	std::list<Branch>::iterator subBranch;
	for (subBranch = branch->branches.begin(); subBranch != branch->branches.end(); ++subBranch)
	{


		if (subBranch->object.p_joint != nullptr)
		{




			float mainBranchAngle = branch->object.p_body->GetAngle();
			float subBranchAngle = subBranch->object.p_body->GetAngle();

			float setpointAngle=  constrainAngle(subBranchAngle - mainBranchAngle);


			float motorSpeed = setpointAngle;//subBranchAngle - mainBranchAngle;

			printf("applying motor speed %f\n", motorSpeed);


			// subBranch->object.p_joint->SetMotorSpeed(motorSpeed);


		}

		recursiveUpdateMotors( &(*subBranch) )  ;
	}
}

void threadGame()
{



	for (int i = 0; i < 1; ++i)
	{
		/* code */

		float xCoord = (RNG() - 0.5) * 100;


		shine(

		    b2Vec2( xCoord , -10 ),
		    b2Vec2( xCoord , 10 )

		);

	}



	// wind = b2Vec2(   clamp(wind.x + (RNG() - 0.5) , -20.0f, 20.0f  ) , clamp(wind.y + (RNG() - 0.5), -5.0f , 5.0f  )  );


	std::list<Tree>::iterator tree;

	std::list<Branch>::iterator branch;
	for (tree = garden.begin(); tree != garden.end(); ++tree)
	{

		// printf("tree energy %f\n", tree->energyStored);

		if (tree->mature)
		{

			Branch * growingBranch = &(tree->branches.back());

			if (tree->flagDelete || (growingBranch->flagDelete)  )  { continue;}
			if ( !(tree->mature) ) 									{ continue;}
			if (growingBranch->seed && tree->germinated) 			{ continue;}
			if (tree->energyStored < 0 ) 							{ continue;}



			grow( &(*tree), growingBranch);
		}


		for (branch = tree->branches.begin(); branch != tree->branches.end(); ++branch)
		{

			recursiveUpdateMotors( &(*branch) );


			if (tree->mature && branch->seed)
			{

				// deleteFromWorld ( &(branch->object) );
				// // tree->branches.erase ( branch );
				// break;
				branch->flagDelete = true;
				branch->object.flagDelete = true;

			}


			// tree->energyStored += branch->capturedLight;
			// branch->capturedLight = 0;

			branch->object.p_body->ApplyForce(wind, branch->object.p_body->GetWorldCenter() , true);



		}

	}

//-----------------

	// delete stuff
	// std::list<Tree>::iterator tree;
	for (tree = garden.begin(); tree != garden.end(); ++tree)
	{

		// std::list<Branch>::iterator branch;
		for (branch = tree->branches.begin(); branch != tree->branches.end(); ++branch)
		{
			if (branch->flagDelete)
			{

				deleteFromWorld ( &(branch->object) );
			}

		}
		tree->branches.remove_if( [](Branch branch)
		{
			return branch.flagDelete;
		}
		                        );


	}


	garden.remove_if( [](Tree tree)
	{
		return tree.flagDelete;
	}
	                );

	physicalObjects.remove_if( [](PhysicalObject * object)
	{
		return object->flagDelete;
	}
	                         );






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

		if ( (*object) == nullptr) {continue;}

		if ( (*object)->p_body == nullptr ) {continue;}

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


		if ( (*object) == nullptr) {continue;}
		if ( (*object)->p_body == nullptr ) {continue;}

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