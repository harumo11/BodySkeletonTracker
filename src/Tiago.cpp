#include <Tiago.h>
#include <stdio.h>
#include <stdlib.h>

Tiago::Tiago() {
	moving = false;
	t = NULL;
	//bzero(centerHV,       sizeof(int)*BUF_SIZE);
}

float Tiago::getAngElbow() {
	return angElbow;
}

float Tiago::getAngShoulder() {
	return angShoulder;
}

void Tiago::setMoving(bool m) {
	moving = m;
}

bool Tiago::isMoving() {
	return moving;
}

void Tiago::setAngElbow(float ang) {
	angShoulder = ang;
}

void Tiago::setAngShoulder(float ang) {
	angElbow = ang;
}

void Tiago::mutexLock() {
	mtx.lock();
}

void Tiago::mutexUnlock() {
	mtx.unlock();
}

void Tiago::moveArmThread() {
	if (moving) return;
	mutexLock();
	//if (t)
	//	delete t;
	//t = new std::thread(&Tiago::moveArm, this);

	int threadId = pthread_create( &thread1, NULL, &Tiago::moveArm, (void *)this);
}



void * Tiago::moveArm(void * t) {
	Tiago * tiago = (Tiago*)t;

	if (tiago->isMoving()) return NULL;

	tiago->setMoving(true);

	char comando[100];
	int joint;
	float ang;
	int r;

	/*if (aShoulder>0) {
		if (aElbow>0)
			ang = aElbow-aShoulder; // subtrai o shoulder do ombro
		else
			ang = aElbow-aShoulder; // soma os dois angulos
	}
	else {
		if (aElbow>0)
			ang = aElbow-aShoulder; // soma os dois angulos
		else
			ang = aElbow-aShoulder; // subtrai o shoulder do ombro
	}*/

	joint = 4;		
	ang = tiago->getAngElbow() - tiago->getAngShoulder();
	ang = ang*ELBOW_90/90.; // conversao do angulo para os valores compativeis com o Tiago.
	// O roscore ou gazebo tem que estar rodando previamente.
	sprintf(comando, "rosrun play_motion move_joint arm_%d_joint %.1f 0.2", joint, ang);
	printf("%s\n", comando);
	r = system(comando);

	joint = 2;
	ang = tiago->getAngShoulder();
	ang = ang*SHOULDER_45/45.; // conversao do angulo para os valores compativeis com o Tiago.
	// O roscore ou gazebo tem que estar rodando previamente.
	sprintf(comando, "rosrun play_motion move_joint arm_%d_joint %.1f 0.2", joint, ang);
	printf("%s\n", comando);
	r = system(comando);

	tiago->setMoving(false);
	tiago->mutexUnlock();
}



void Tiago::detectTiagoCommands(SkeletonPoints* sp, int afa) {
	static int c=0;
	c++;

	// mao esquerda esticada e afastada do corpo, comandos ativados.
	if (sp->leftHand.x!=0 && sp->leftHand.x < sp->bodyPoints[SkeletonPoints::HEAD]->x - afa*2 && sp->leftHand.y > sp->getCenterY()+afa)
	{
		// Tronco
		int head_1 = sp->vHead[SkeletonPoints::HEAD] -1 >= 0 ? sp->vHead[SkeletonPoints::HEAD]-1 : BUF_SIZE-1;
		
		int y1 = sp->pointsV[SkeletonPoints::HEAD][ head_1    % BUF_SIZE].y;
		int y2 = sp->pointsV[SkeletonPoints::HEAD][(head_1+1) % BUF_SIZE].y;
		//printf("%d::Recebendo comandos (%d - %d)=%d\n", c++, y1, y2, y1 - y2);
		if (y1 - y2 > 20)
			printf("%d::TRONCO para BAIXO\n", c);
		if (y1 - y2 < -20)
			printf("%d::TRONCO para CIMA\n", c);


		// Braco // TODO verificar se o rightShoulder.y-middleArmRight.y eh menor ou maior.
		// O y do mais baixo e do mais a direita e o do middleArm forem muito proximos. E o y do ombro longe (caso do braco todo esticado na linha do ombro). E distante da mao
/*		if (middleArmRight.x!=0 && (abs(maxRight.y-maxBottomRight.y)<15 && abs(maxRight.y-middleArmRight.y)<15 && abs(rightShoulder.y-middleArmRight.y)<15 &&
		euclideanDist(middleArmRight, sp->rightHand)>50 )) {
			printf("BRACO ESTICADO HORIZONTAL\n");
		}*/


		// so entra a cada 10c para nao poluir muito o terminal	
		//if (c%10==0)
		{
			float angShoulder, angElbow;
			// Angulo entre ombro e cotovelo
			if (sp->rightHand.x!=0 && sp->rightElbow.x!=0) {
				angShoulder = -atan2f(sp->rightElbow.y-sp->rightShoulder.y, sp->rightElbow.x-sp->rightShoulder.x)*180/CV_PI;
				angShoulder = (((int)angShoulder)/5)*5;
				setAngShoulder(angShoulder);
				//printf("ANG::COTOVELO::OMBRO::%.1f\n", angShoulder);
			}

			// Angulo entre antebraco e cotovelo
			if (sp->rightHand.x!=0 && sp->rightElbow.x!=0) {
				angElbow = -atan2f(sp->rightHand.y-sp->rightElbow.y, sp->rightHand.x-sp->rightElbow.x)*180/CV_PI;
				angElbow = (((int)angElbow)/5)*5;
				setAngElbow(angElbow);
				//printf("ANG::COTOVELO:: MAO ::%.1f\n\n", angElbow);
			}

			// TODO descomentar
			//tiago->moveArmThread();
		}

	}

	// so entra a cada 10c para nao poluir muito o terminal	
	if (c%10==0)
		// se a mao esquerda estiver mais a esquerda do que o ombro, e ambos estiverem acima da linha da cintura
		if (sp->leftHand.x!=0 && sp->leftElbow.x!=0 && sp->leftHand.y < sp->getCenterY()-afa && sp->leftElbow.y < sp->getCenterY() )
		{
			if (sp->leftHand.x - sp->leftElbow.x < -25)
				printf("andar para frente\n");
			else if (sp->leftHand.x - sp->leftElbow.x > 20)
				printf("andar para tras\n");
		}
}



