#include "physics.h"
#include "graphics.h"
#include "menus.h"
#include "main.h"

int exampleNumberCapture = 15;
std::string exampleTextCapture = std::string("exampleText");

b2Vec2 wind = b2Vec2(0.0f, 0.0f);

unsigned long int ticks = 0;

Branch::Branch(float rootThickness, float tipThickness, float length, float naturalAngle, b2Color color )
{
	this->owner = nullptr;
	this-> energyValue = ((rootThickness + tipThickness) / 2) * length;
	this->color = color;
	this->rootThickness = rootThickness;
	this->tipThickness = tipThickness;
	this->length = length;
	this->naturalAngle  = naturalAngle;
	this-> branches = std::list<Branch>();
	this->capturedLight = 0;
	this-> vertices =
	{
		b2Vec2( + (this->rootThickness / 2),  -(this->length / 2)),
		b2Vec2( - (this->rootThickness / 2),  -(this->length / 2)),
		b2Vec2( - (this->tipThickness / 2),   +(this->length / 2)),
		b2Vec2( + (this->tipThickness / 2),   +(this->length / 2))
	};
	this->seed = false;
	this->object = PhysicalObject(this->vertices, false);
	this->flagDelete = false;
	this->stem = false;
	this->rootThicknessDelta = 0.0f 		;
	this->tipThicknessDelta  = 0.0f 		;
	this->lengthDelta 		= 0.0f 		;
	this->angleDelta		= 0.0f 		;
	this->expressed = false;
	this->geneCursor = 0;
	this->color = b2Color(0.0f, 0.0f, 0.0f, 1.0f)	;

	this->rjointDef = b2RevoluteJointDef();
	this->p_rjoint = nullptr;

	this->djointDef = b2DistanceJointDef();
	this->p_djoint = nullptr;

	this->wjointDef = b2WeldJointDef();
	this->p_wjoint = nullptr;
}

Tree::Tree(std::string genes)
{
	this->flagDelete = false;
	this-> energyStored = 1;
	this-> genes = genes;
	printf("new tree: %s\n", genes.c_str());
	this-> lastReproduced = ticks;
	this->mature = false;
	this->  branches = std::list<Branch>();
	this->germinated = false;
	this->sproutPosition = b2Vec2(0.0f, 0.0f);
	this->affixedObject = nullptr;
}

std::list<Tree> garden;
std::list<PhysicalObject> terrain;

void instantiateSeed (std::string genes, Branch * joinBranch, b2Vec2 position)
{
	garden.push_back(  Tree(genes)  );
	Tree * tree = &(garden.back()) ;
	tree->branches.push_back( Branch(1.0f, 1.0f, 1.0f, 0.0f, b2Color(0.8f, 0.8f, 0.1f, 1.0f)  ) );
	Branch * newBranch = &(tree->branches.back());

	newBranch->geneCursorStartPosition = 0;
	newBranch->geneCursor = 0;

	newBranch->seed = true;
	tree->mature = false;
	tree->germinated = false;
	b2Vec2 newBranchPosition ;

	if (joinBranch != nullptr)
	{
		newBranchPosition = b2Vec2(

		                        joinBranch ->object.p_body->GetWorldCenter() .x
		                        ,
		                        joinBranch ->object.p_body->GetWorldCenter() .y
		                    );
	}

	else {
		newBranchPosition = b2Vec2(position.x, position.y);
	}

	addToWorld( &(newBranch->object), newBranchPosition , 0.0f  );
	newBranch->color = b2Color(0.8f, 0.8f, 0.1f, 1.0f) ;
	newBranch->object.owner = newBranch;
	newBranch->owner = tree;
	if (joinBranch != nullptr)
	{
	}
}


Branch * addBranchSegment (Tree * tree, Branch * growingBranch, float absoluteAngleOffset)
{
	Branch * newBranch ;
	float newBranch_length        = clamp ( abs(growingBranch->length 			+ growingBranch->lengthDelta 		), 0.1f, 10.0f	);
	float newBranch_tipThickness  = clamp ( abs(growingBranch->tipThickness 	+ growingBranch->tipThicknessDelta	), 0.1f, 10.0f	);
	float newBranch_rootThickness = clamp ( abs(growingBranch->rootThickness 	+ growingBranch->rootThicknessDelta	), 0.1f, 10.0f	);
	float newBranch_naturalAngle  = constrainAngle(growingBranch->naturalAngle 	+ growingBranch->angleDelta			);
	b2Color newBranch_color = b2Color(
	                              growingBranch->color.r + growingBranch->colorDelta.r,
	                              growingBranch->color.g + growingBranch->colorDelta.g,
	                              growingBranch->color.b + growingBranch->colorDelta.b
	                          );

	if (growingBranch->seed)
	{
		newBranch_color = b2Color(0.0f, 0.0f, 0.0f, 1.0f);
		tree->branches.push_back( Branch( newBranch_rootThickness, newBranch_tipThickness, newBranch_length, 0.0f, newBranch_color  ) );
		newBranch = &(tree->branches.back());
		if (tree->mature && !tree->germinated)
		{
			tree->germinated = true;
		}
	}
	else
	{
		growingBranch->branches.push_back( Branch( newBranch_rootThickness, newBranch_tipThickness, newBranch_length, newBranch_naturalAngle, newBranch_color ) );
		newBranch = &(growingBranch->branches.back());
	}

	newBranch->length = newBranch_length;
	newBranch->tipThickness = newBranch_tipThickness;
	newBranch->rootThickness = newBranch_rootThickness;
	newBranch->naturalAngle = newBranch_naturalAngle ;
	newBranch->color = newBranch_color;

	float actualAngle = constrainAngle( newBranch->naturalAngle + growingBranch->object.p_body->GetAngle() + absoluteAngleOffset );

	printf("actual angle = %f\n", actualAngle);
	b2Vec2 newBranchPosition = b2Vec2(
	                               growingBranch->object.p_body->GetWorldCenter().x + (  (growingBranch->length / 2) * cos(growingBranch->object.p_body->GetAngle() )   )   ,
	                               growingBranch->object.p_body->GetWorldCenter().y + (  (growingBranch->length / 2) * sin(growingBranch->object.p_body->GetAngle() )    )
	                           );

	newBranchPosition = b2Vec2(
	                        newBranchPosition.x +  (newBranch->length / 2) * cos(actualAngle)
	                        ,
	                        newBranchPosition.y +  (newBranch->length / 2) * sin(actualAngle)
	                    );

	newBranch->object.owner = newBranch;
	newBranch->owner = tree;
	newBranch->geneCursorStartPosition = growingBranch->geneCursor;
	newBranch->geneCursor = newBranch->geneCursorStartPosition;
	tree->energyStored -= (( newBranch->rootThickness + newBranch->tipThickness ) / 2 ) * newBranch->length ;
	addToWorld( &(newBranch->object), newBranchPosition , actualAngle  );
	if (
	    growingBranch != nullptr
	)
	{
		if (
		    growingBranch != newBranch
		)
		{
			if (
			    growingBranch->object.p_body != nullptr
			)
			{
				if (  (!( growingBranch->seed ))   )
				{
					createJoint( newBranch, growingBranch , newBranch->naturalAngle);
				}

				if (growingBranch->seed)
				{
					if (tree->affixedObject != nullptr)
					{
						printf("a tree grew in the ground\n");
						b2Vec2 bLocalAnchor = b2Vec2(
						                          tree->sproutPosition.x - tree->affixedObject->p_body->GetWorldCenter().x ,
						                          tree->sproutPosition.y - tree->affixedObject->p_body->GetWorldCenter().y
						                      );

						newBranch->object.p_body ->SetTransform(tree->sproutPosition, 0.5f * const_pi);
						newBranch->object.p_body ->SetLinearVelocity( b2Vec2(0.0f, 0.0f) );
						createJointWithVariableBAnchor( newBranch, tree->affixedObject , bLocalAnchor );
						newBranch->stem = true;
					}
				}
			}
		}
	}
	return newBranch;
}

Branch * transcribeNextSegments (Tree * tree , Branch * growingBranch, float absoluteAngleOffset)
{
	printf("transcribeNextSegments\n");
	printf("geneCursor %u, genome length %lu\n", growingBranch->geneCursor, tree->genes.length()  );

	bool drawnAnythingYet = false;

	while (growingBranch->geneCursor < tree->genes.length() )
	{

		if (growingBranch->geneCursor == 0) {growingBranch-> geneCursor++; continue;}
		switch (tree->genes[growingBranch->geneCursor])
		{
		case 's':
		{
			// serial array
			growingBranch-> geneCursor++;
			int arrayN = alphanumeric(tree->genes[growingBranch->geneCursor]);
			printf("sequence %i\n", arrayN);
			Branch * arrayBranch = growingBranch;
			unsigned int arrayStartGeneCursor = growingBranch->geneCursor;
			for (int i = 0; i < arrayN; ++i)
			{
				printf(",");
				if (arrayBranch != nullptr)
				{
					arrayBranch->geneCursor = arrayStartGeneCursor;
					arrayBranch = transcribeNextSegments(tree, arrayBranch, 0.0f);
				}
			}
			return arrayBranch;
			break;
		}
		case 'q':
		{
			// parallel array with angle
			growingBranch-> geneCursor++;
			int arrayN = alphanumeric(tree->genes[growingBranch->geneCursor]);
			growingBranch-> geneCursor++;
			float alpha = alphanumeric(tree->genes[growingBranch->geneCursor]);
			float angleAdjust = ((alpha / 26) * 2 * const_pi );

			printf("array %i\n", arrayN);

			unsigned int arrayStartGeneCursor = growingBranch->geneCursor;
			float anglePerUnit = (angleAdjust / arrayN);
			float startingAngle = (angleAdjust / 2) ;

			for (int i = 0; i < arrayN; ++i)
			{
				growingBranch->geneCursor = arrayStartGeneCursor;
				transcribeNextSegments(tree, growingBranch, (anglePerUnit * i) - startingAngle )  ;
			}
		}
		case ' ':
		{
			growingBranch-> geneCursor++;
			// end array or sequence
			printf("break\n");
			if (drawnAnythingYet) {return nullptr;}
			break;

		}
		case 'f':
		{
			growingBranch-> geneCursor++;
			// draw a seed
			printf("seed\n");
			instantiateSeed(tree->genes, growingBranch, b2Vec2(50, 50));
			drawnAnythingYet = true;
			return nullptr;
		}
		case 'b':
		{
			growingBranch-> geneCursor++;
			printf("add segment\n");
			// draw a new branch segment
			Branch * newBranch = addBranchSegment ( tree, growingBranch, absoluteAngleOffset) ;
			drawnAnythingYet = true;
			return newBranch;
		}
		case 'a':
		{
			// increment angle cursor
			growingBranch-> geneCursor++;
			float alpha = alphanumeric(tree->genes[growingBranch->geneCursor]);
			float angleAdjust = (alpha - 13) / 10;
			printf("increment angle %f\n", angleAdjust);
			growingBranch->angleDelta += angleAdjust;
			break;
		}
		case 'r':
		{
			// increment root thickness cursor
			growingBranch-> geneCursor++;
			float alpha = alphanumeric(tree->genes[growingBranch->geneCursor]);
			float rootAdjust = (alpha - 13) / 10;
			printf("increment root thickness %f\n", rootAdjust);
			growingBranch->rootThicknessDelta += rootAdjust;
			break;
		}
		case 't':
		{
			// increment tip thickness cursor
			growingBranch-> geneCursor++;
			float alpha = alphanumeric(tree->genes[growingBranch->geneCursor]);
			float tipAdjust = (alpha - 13) / 10;
			printf("increment tip thickness %f\n", tipAdjust);
			growingBranch->tipThicknessDelta += tipAdjust;
			break;
		}
		case 'l':
		{
			// increment length cursor
			growingBranch-> geneCursor++;
			float alpha = alphanumeric(tree->genes[growingBranch->geneCursor]);
			float lengthAdjust = (alpha - 13) / 10;
			printf("increment length %f\n", lengthAdjust);
			growingBranch->lengthDelta += lengthAdjust;
			break;
		}
		default:
		{
			growingBranch-> geneCursor++;
			printf(".");
			break;
		}
		}
	}
	return nullptr;
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
	std::vector<b2Vec2> exampleBox2Vertices =
	{
		b2Vec2( +100 * exampleBoxSize ,  -1 * exampleBoxSize), //b2Vec2 rootVertexA =
		b2Vec2( -100 * exampleBoxSize ,  -1 * exampleBoxSize), // b2Vec2 rootVertexB =
		b2Vec2( -100 * exampleBoxSize ,  +1 * exampleBoxSize), //b2Vec2 tipVertexA =
		b2Vec2( +100 * exampleBoxSize ,  +1 * exampleBoxSize) // b2Vec2 tipVertexB =
	};

	terrain.push_back( PhysicalObject(exampleBox2Vertices, true) );

	addToWorld( &(terrain.back()) , b2Vec2(0.0f, -20.0f), 0.0f );

	// std::string exampleSentence = std::string("Weld joint definition. You need to specify local anchor points where they are attached and the relative body angle. The position of the anchor points is important for computing the reaction torque. ");
	std::string exampleSentence = std::string(" bbbbbbbbbbbbbbbbbbqdjbbbbbf ");
	instantiateSeed(exampleSentence, nullptr, b2Vec2(50, 50));
}

void recursiveUpdateMotors( Branch * branch )
{
	if (branch->stem)
	{
		if (branch->p_rjoint != nullptr)
		{

			float motorAngle = branch->p_rjoint->GetJointAngle();
			// printf("motor angle %f\n", motorAngle);


			float motorSpeed = constrainAngle((motorAngle + branch->naturalAngle )  )   ;
			motorSpeed += branch->p_rjoint->GetJointSpeed() * -0.5;
			branch->p_rjoint->SetMotorSpeed(motorSpeed * 0.001);
		}
	}

	std::list<Branch>::iterator subBranch;
	for (subBranch = branch->branches.begin(); subBranch != branch->branches.end(); ++subBranch)
	{
		if (subBranch->p_rjoint != nullptr)
		{

			float motorAngle = subBranch->p_rjoint->GetJointAngle();
			float motorSpeed = constrainAngle((motorAngle ) + (1.0f * const_pi) +  branch->naturalAngle)  ;
			motorSpeed += subBranch->p_rjoint->GetJointSpeed() * -0.5;
			subBranch->p_rjoint->SetMotorSpeed(motorSpeed * 0.001);
		}
		recursiveUpdateMotors( &(*subBranch) )  ;
	}
}
void recursiveGrow (Tree * tree, Branch * growingBranch)
{
	if (!growingBranch->expressed)
	{
		Branch * tempBranch = transcribeNextSegments(tree, growingBranch, 0.0f);
		growingBranch->expressed = true;
	}
	std::list<Branch>::iterator branch;
	for (branch = growingBranch->branches.begin(); branch != growingBranch->branches.end(); ++branch)
	{
		recursiveGrow ( tree, &(*branch));
	}
}

void threadGame()
{
	for (int i = 0; i < 1; ++i)
	{
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
		for (branch = tree->branches.begin(); branch != tree->branches.end(); ++branch)
		{
			if (tree->mature )
			{
				recursiveGrow (&(*tree), &(*branch));
			}
			recursiveUpdateMotors( &(*branch) );
			if (tree->mature && branch->seed)
			{
				branch->flagDelete = true;
				branch->object.flagDelete = true;
			}


			if (branch->seed)
			{

				branch->object.p_body->ApplyForce(b2Vec2(0.0, -1.0f), branch->object.p_body->GetWorldCenter() , true);
			}

		}
	}

	// delete stuff
	for (tree = garden.begin(); tree != garden.end(); ++tree)
	{
		for (branch = tree->branches.begin(); branch != tree->branches.end(); ++branch)
		{
			if (branch->flagDelete)
			{
				if (branch->p_rjoint != nullptr)
				{
					m_world->DestroyJoint(branch->p_rjoint);
				}
				if (branch->p_djoint != nullptr)
				{

					m_world->DestroyJoint(branch->p_djoint);
				}
				if (branch->p_wjoint != nullptr)
				{

					m_world->DestroyJoint(branch->p_wjoint);
				}
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

unsigned int recursiveDrawingAssessment (Branch * branch)
{
	unsigned int total = 1;

	std::list<Branch>::iterator subBranch;
	for (subBranch = branch->branches.begin(); subBranch != branch->branches.end(); ++subBranch)
	{
		total += recursiveDrawingAssessment( &(*subBranch));
	}

	return total;
}

void drawPhysicalObject (
    PhysicalObject * object,
    unsigned int * vertex_buffer_cursor, float * vertex_buffer_data,
    unsigned int * index_buffer_cursor,  unsigned int * index_buffer_content, unsigned int * index_buffer_data )
{

	if ( object->p_body != nullptr )
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

			// printf("bodyposition %f 5f")
			rotatedPoint = b2RotatePointPrecomputed( bodyPosition, bodyAngleSin, bodyAngleCos, rotatedPoint);

			vertex_buffer_data[(*vertex_buffer_cursor) + 0] = object->color.r;
			vertex_buffer_data[(*vertex_buffer_cursor) + 1] = object->color.g;
			vertex_buffer_data[(*vertex_buffer_cursor) + 2] = object->color.b;
			vertex_buffer_data[(*vertex_buffer_cursor) + 3] = object->color.a;
			vertex_buffer_data[(*vertex_buffer_cursor) + 4] = rotatedPoint.x;
			vertex_buffer_data[(*vertex_buffer_cursor) + 5] = rotatedPoint.y ;
			(*vertex_buffer_cursor) += 6;

			index_buffer_data[(*index_buffer_cursor)] = (*index_buffer_content);
			(*index_buffer_cursor)++;
			(*index_buffer_content)++;
		}

		index_buffer_data[(*index_buffer_cursor)] = PRIMITIVE_RESTART;
		(*index_buffer_cursor)++;
	}
}

void recursiveDraw (Branch * branch,
                    unsigned int * vertex_buffer_cursor, float * vertex_buffer_data,
                    unsigned int * index_buffer_cursor,  unsigned int * index_buffer_content, unsigned int * index_buffer_data )
{
	drawPhysicalObject( &(branch->object), vertex_buffer_cursor, vertex_buffer_data, index_buffer_cursor, index_buffer_content, index_buffer_data );

	std::list<Branch>::iterator subBranch;
	for (subBranch = branch->branches.begin(); subBranch != branch->branches.end(); ++subBranch)
	{
		recursiveDraw( &(*subBranch)  , vertex_buffer_cursor, vertex_buffer_data, index_buffer_cursor, index_buffer_content, index_buffer_data  );
	}
}

void gameGraphics()
{
	/** your graphics logic here. turn your data into floats and pack it into vertex_buffer_data. The sequence is r, g, b, a, x, y; repeat for each point. **/
	unsigned int numberOfSquaresToDraw = 0;
	std::list<Tree>::iterator tree;
	std::list<Branch>::iterator branch;
	for (tree = garden.begin(); tree != garden.end(); ++tree)
	{
		for (branch = tree->branches.begin(); branch != tree->branches.end(); ++branch)
		{
			numberOfSquaresToDraw += recursiveDrawingAssessment( &(*branch) );
		}
	}

	std::list<PhysicalObject>::iterator rock;
	for (rock = terrain.begin(); rock != terrain.end(); ++rock)
	{
		numberOfSquaresToDraw++;
	}

	unsigned int nVertsToRenderThisTurn = numberOfSquaresToDraw * 4;
	unsigned int nIndicesToUseThisTurn = numberOfSquaresToDraw * 5;
	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
	unsigned int vertex_buffer_cursor = 0;
	float vertex_buffer_data[totalNumberOfFields];
	unsigned int index_buffer_cursor = 0;
	unsigned int index_buffer_content = 0;
	unsigned int index_buffer_data[nIndicesToUseThisTurn];


	for (tree = garden.begin(); tree != garden.end(); ++tree)
	{
		for (branch = tree->branches.begin(); branch != tree->branches.end(); ++branch)
		{
			recursiveDraw ( &(*branch) , &vertex_buffer_cursor, vertex_buffer_data, &index_buffer_cursor, &index_buffer_content, index_buffer_data );
		}
	}
	for (rock = terrain.begin(); rock != terrain.end(); ++rock)
	{
		// numberOfSquaresToDraw++;

		drawPhysicalObject( &(*rock), &vertex_buffer_cursor, vertex_buffer_data, &index_buffer_cursor, &index_buffer_content, index_buffer_data );


	}

	glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_buffer_data ), vertex_buffer_data, GL_DYNAMIC_DRAW );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_DYNAMIC_DRAW);
	glDrawElements( GL_TRIANGLE_FAN, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );
}