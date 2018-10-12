#include	"petha-knife.h"

#define _USE_MATH_DEFINES
#include <math.h>

MicronTracker	mTracker;
std::vector<MARKER>	marker;
RelativePosition metz;
std::vector<Position> tips;

// ���X��[�ʒu (���L�������[��p)
SHMEM_KNIFE* shmem_knife;

// �L�����u���[�V�����}�g���b�N�X
// double�^��4x4�s��m�ہC0�ŏ�����
cv::Mat m_c = cv::Mat::zeros(4, 4, CV_64F);

double height = 43;		//�}�[�J�[���璆�S�ւ̍���
double length = 325;	//���S�����[�܂ł̒���

auto metzMARKER::operator=(MARKER &m) {

	this->Hazard = m.Hazard;
	this->isIdentified = m.isIdentified;
	this->name = m.name;
	for (auto i = 0; i < 3; ++i) {
		this->pos[i] = m.pos[i];
		this->ang[i] = m.ang[i];
	}
	for (auto i = 0; i < 9; ++i) {
		this->rot[i] = m.rot[i];
	}

	return *this;

}


bool Position::operator==(const metzMARKER &m) const {

	return this->name == m.name;

}

void Position::set(double pos[3]) {

	x = pos[0];
	y = pos[1];
	z = pos[2];

}


void RelativePosition::setMarker(std::vector<MARKER> &marker) {

	this->m.clear();
	this->m.shrink_to_fit();

	for (auto m : marker) {

		metzMARKER metz;
		metz = m;

		this->m.push_back(metz);

	}

}



/**
* �}�[�J�����w�肵�Č���
*/
std::vector<metzMARKER> RelativePosition::find(std::string name) {

	std::vector<metzMARKER> mBuff;

	for (auto m : this->m) {
		if (m.name == name) mBuff.push_back(m);
	}

	return mBuff;

}

/**
* �������擪�Ɍ��������������ʂ��擾
*/
metzMARKER RelativePosition::getFindTop(std::string name) {

	std::vector<metzMARKER> mBuff = find(name);

	if (mBuff.size() >= 1) {
		return mBuff.at(0);
	}

	return metzMARKER();

}

/**
* ���Έʒu�x�N�g�������߂�֐�
* @param �}�[�J�[A�̖���
* @param �}�[�J�[B�̖���
* @Note vector 02_01
*/
bool RelativePosition::relativePosVector(std::string mAName, std::string mBName) {

	if (find(mAName).size() > 1 || find(mBName).size() > 1) {
		std::cerr << "[�x��] ���ꖼ�̂̃}�[�J�𕡐����o���܂����D" << std::endl;
	}

	metzMARKER mA = getFindTop(mAName);
	metzMARKER mB = getFindTop(mBName);

	if (mA.isIdentified == true && mB.isIdentified == true) {

		// ��_c�ł�mA����mB�ւ̑��Έʒu�x�N�g��
		double pc_rel[3];	// p_rel^c
		for (auto i = 0; i < 3; ++i) {
			// p_rel^c = p_table^c - p_knife^c
			pc_rel[i] = mA.pos[i] - mB.pos[i];
		}

		// p_rel^k = r_knife^c^-1 * p_rel^c
		double relPosVec[3];
		relPosVec[0] = (pc_rel[0] * mB.rot[0] + pc_rel[1] * mB.rot[1] + pc_rel[2] * mB.rot[2]);
		relPosVec[1] = (pc_rel[0] * mB.rot[3] + pc_rel[1] * mB.rot[4] + pc_rel[2] * mB.rot[5]);
		relPosVec[2] = (pc_rel[0] * mB.rot[6] + pc_rel[1] * mB.rot[7] + pc_rel[2] * mB.rot[8]);

		auto it = std::find(this->relPosVecs.begin(), this->relPosVecs.end(), mB);

		// ���ɁA���Έʒu�x�N�g�������݂���ꍇ
		if (it != this->relPosVecs.end()) {

			it._Ptr->set(relPosVec);

			std::cout << mBName << "����̑��Έʒu�x�N�g�����X�V���܂����D" << std::endl;

		}
		else {

			this->relPosVecs.push_back(Position(mBName, relPosVec));

			std::cout << mBName << "����̑��Έʒu�x�N�g����V�K�o�^���܂����D" << std::endl;

		}

		return true;

	}

	return false;

}

/**
* ��[���W�̐�����s��
*/
std::vector<Position> RelativePosition::getTips() {

	std::vector<Position> tips;

	for (auto m : this->m) {
		if (m.name == TIP_MARKER_NAME) continue;
		// �}�[�J���F�����Ă���ꍇ
		if (m.isIdentified == true && relPosVecs.size() > 0) {

			auto it = std::find(relPosVecs.begin(), relPosVecs.end(), getFindTop(m.name))._Ptr;

			Position tip;
			/*
			// p(c)_tip = p(c)_rel * r(c)_knife + p(c)_knife
			tip.x = it->x * m.rot[0] + it->y * m.rot[3] + it->z * m.rot[6] + m.pos[0];
			tip.y = it->x * m.rot[1] + it->y * m.rot[4] + it->z * m.rot[7] + m.pos[1];
			tip.z = it->x * m.rot[2] + it->y * m.rot[5] + it->z * m.rot[8] + m.pos[2];
			
			tip.name = m.name;
			*/

			tips.push_back(tip);

		}

	}

	return tips;

}

/**
* �A�C�h���R�[���o�b�N�֐�
*/
void idle()
{
	glutPostRedisplay();
}


/**
* Glut ����������
*/
void initGL()
{
	// �����̐ݒ�
	GLfloat lpos0[4] = { 300.0, 300.0, 300.0, 1.0 };
	GLfloat lcol0[4] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lpos0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lcol0);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, lcol0);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);

	// �w�i�F
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClearColor(1.0, 1.0, 1.0, 1.0);  

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
}


/**
* �E�B���h�E�T�C�Y�̃��T�C�Y�֐�
*/
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLdouble)w / h, 1.0, 2000.0);
	glMatrixMode(GL_MODELVIEW);
}


/**
* ���W���̕`����s���֐�
* @param �{��
* @param ����
*/
void drawAxis(double scale, double len)
{

	glPushMatrix();
	{
		glScaled(scale, scale, scale);

		// x Axis
		glPushMatrix();
		{
			GLfloat col[4] = { 1.0, 0.0, 0.0, 1.0 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
			glTranslated(len / 2.0, 0.0, 0.0);
			glScaled(len, 1.0, 1.0);
			glutSolidCube(1.0);
		}
		glPopMatrix();

		// y Axis
		glPushMatrix();
		{
			GLfloat col[4] = { 0.0, 1.0, 0.0, 1.0 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
			glTranslated(0.0, len / 2.0, 0.0);
			glScaled(1.0, len, 1.0);
			glutSolidCube(1.0);
		}
		glPopMatrix();

		// z Axis
		glPushMatrix();
		{
			GLfloat col[4] = { 0.0, 0.0, 1.0, 1.0 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
			glTranslated(0.0, 0.0, len / 2.0);
			glScaled(1.0, 1.0, len);
			glutSolidCube(1.0);
		}
		glPopMatrix();

	}
	glPopMatrix();

}


/**
* �}�[�J�̕`����s��
* @Note: �J�����Ɏʂ��Ă���S�Ẵ}�[�J��\������
* @param �}�[�J���
* @param �\���F
*/
void viewMarker(std::vector<MARKER> m, GLfloat col[4]) {

	for (auto &marc : m) {

		if (marc.isIdentified == true) {
			glPushMatrix();
			{
				glTranslated(marc.pos[0], marc.pos[1], marc.pos[2]);
				glRotated(marc.ang[2], 0.0, 0.0, 1.0);
				glRotated(marc.ang[1], 0.0, 1.0, 0.0);
				glRotated(marc.ang[0], 1.0, 0.0, 0.0);

				glScaled(10.0, 10.0, 1.0);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
				glutSolidCube(1.0);
				//std::cout << marc.name << std::endl;
			}
			glPopMatrix();
		}
	}
}


/**
* �L�[�{�[�h���͏����֐�
* @Note: ���X��[�ʒu�̐�����s��
*/
void keyboard(unsigned char k, int x, int y)
{

	switch (k)
	{
	case ' ':	// �L�����u���[�V�������s

		for (auto n : BODY_MARKER_NAME)
			// ���΃x�N�g�����Z�o
			metz.relativePosVector(TIP_MARKER_NAME, n);
		break;

	case 'a':
		for (auto &marc : marker) {
			if (marc.name == "01") {
				for (int i = 0; i < 3; i++) {
					shmem_knife->s_pos[i] = marc.pos[i];
					//std::cout << shmem_knife->s_pos[i] << ", ";
				}
				//std::cout << std::endl;
			}
		}
		break;
	case 's':
		std::cout << shmem_knife->x << ", " << shmem_knife->y << ", " << shmem_knife->z << std::endl;
		std::cout << shmem_knife->xt << ", " << shmem_knife->yt << ", " << shmem_knife->zt << std::endl;
		break;
	case 'q':	// �I������

		exit(0);
		break;

	default:

		break;

	}

}


/**
* �`��֐��i���[�v���[�`���j
*/
void display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, -10.0,	// ���_�ʒu
		0.0, 0.0, 0.0,			// ���_�ڕW�ʒu
		0.0, -1.0, 0.0);		// ������x�N�g��

								// ���W����`��
								//drawAxis(0.1, 100);

								// MicronTracker��Marker���擾
	marker = mTracker.getMicronTrackerMarker();

	// �J�����Ɏʂ��Ă��邷�ׂẴ}�[�J��\��
	GLfloat col[4] = { 0.0, 1.0, 0.0, 1.0 };
	viewMarker(marker, col);

	// ���X��[�ʒu�̐���
	metz.setMarker(marker);
	tips = metz.getTips();

	/**/
	std::vector<double> tipAngle_x;
	std::vector<double> tipAngle_y;
	std::vector<double> tipAngle_z;
	//02
	for (auto &marc : marker) {
		if (marc.name == "02") {
			//std::cout << "2 " << marc.ang[0] << std::endl;
			tipAngle_x.push_back(marc.ang[0]);
			tipAngle_y.push_back(marc.ang[1]);
			tipAngle_z.push_back(marc.ang[2]);
		}
	}


	// 03
	for (auto &marc2 : marker) {
		if (marc2.name == "03") {
			//std::cout << "3 " << marc2.ang[0]  << std::endl;
			//std::cout << "sum " << marc.ang[0] + marc2.ang[0] << std::endl;
			//	std::cout << std::endl;
			tipAngle_x.push_back(marc2.ang[0] + 60);
			tipAngle_y.push_back(marc2.ang[1]);
			tipAngle_z.push_back(marc2.ang[2]);
		}
	}

	// 04
	for (auto &marc3 : marker) {
		if (marc3.name == "04") {

			//std::cout << "4 " << marc3.ang[0]  << std::endl;
			//std::cout << "sum " << marc.ang[0] + marc2.ang[0] << std::endl;
			//	std::cout << std::endl;
			tipAngle_x.push_back(marc3.ang[0] + 120);
			tipAngle_y.push_back(marc3.ang[1]);
			tipAngle_z.push_back(marc3.ang[2]);
		}
	}
	//05
	/*
	for (auto &marc4 : marker) {
	if (marc4.name == "05" && "04" && "06") {

	//std::cout << "5 " << marc4.ang[0]  << std::endl;

	//std::cout << "sum " << marc.ang[0] + marc2.ang[0] << std::endl;
	//	std::cout << std::endl;
	//tipAngle_x.push_back(marc4.ang[0] - 180);
	//tipAngle_x.push_back(marc4.ang[0] + 180);
	tipAngle_y.push_back(marc4.ang[1]);
	tipAngle_z.push_back(marc4.ang[2]);
	}
	*/

	for (auto &marc4 : marker) {
		if (marc4.name == "05") {
			if (marc4.ang[0] > 0) {
				tipAngle_x.push_back(-180 + marc4.ang[0]);
			}
			else {
				tipAngle_x.push_back(180 + marc4.ang[0]);
			}

			tipAngle_y.push_back(marc4.ang[1]);
			tipAngle_z.push_back(marc4.ang[2]);
		}
	}

	//std::cout << shmem_knife->xt << ", " << shmem_knife->yt << ", " << shmem_knife->zt << std::endl;
	//06
	for (auto &marc5 : marker) {
		if (marc5.name == "06") {

			//std::cout << "6 " << marc5.ang[0]  << std::endl;

			//std::cout << "sum " << marc.ang[0] + marc2.ang[0] << std::endl;
			//	std::cout << std::endl;
			tipAngle_x.push_back(marc5.ang[0] - 120);
			tipAngle_y.push_back(marc5.ang[1]);
			tipAngle_z.push_back(marc5.ang[2]);
		}
	}

	//06
	for (auto &marc6 : marker) {
		if (marc6.name == "07") {

			//std::cout << "7 " << marc6.ang[0] << std::endl;

			//std::cout << "sum " << marc.ang[0] + marc2.ang[0] << std::endl;
			//	std::cout << std::endl;
			tipAngle_x.push_back(marc6.ang[0] - 60);
			tipAngle_y.push_back(marc6.ang[1]);
			tipAngle_z.push_back(marc6.ang[2]);
		}
	}

	double sumTipsAngle_x = 0.0f;
	for (auto t : tipAngle_x) {
		sumTipsAngle_x += t;
	}
	double sumTipsAngle_y = 0.0f;
	for (auto t : tipAngle_y) {
		sumTipsAngle_y += t;
	}
	double sumTipsAngle_z = 0.0f;
	for (auto t : tipAngle_z) {
		sumTipsAngle_z += t;
	}

	std::vector<double> tip_x;
	std::vector<double> tip_y;
	std::vector<double> tip_z;
	for (auto &m : marker) {
		//std::cout << m.name << " : " << m.pos[0] << ", " << m.pos[1] << ", " << m.pos[2] << std::endl;
		//std::cout << m.name << " : " << m.ang[0] << ", " << m.ang[1] << ", " << m.ang[2] << std::endl;
		if (m.name == "01") continue;
		double cx, cy, cz;
		/*
		x = 43 * cos((m.ang[1] + 90) * M_PI / 180) + m.pos[0];
		y = -43 * sin((m.ang[1] + 90 ) * M_PI / 180) * sin(m.ang[0] * M_PI / 180) + m.pos[1];
		z = 43 * sin((m.ang[1] + 90 ) * M_PI / 180) * cos(m.ang[0] * M_PI / 180) + m.pos[2];
		*/

		//���S�ʒu����
		cx = height * cos(m.ang[0] * M_PI / 180) * sin(m.ang[1] * M_PI / 180) * cos(m.ang[2] * M_PI / 180) + height * sin(m.ang[0] * M_PI / 180) * sin(m.ang[2] * M_PI / 180) + m.pos[0];
		cy = height * sin(-m.ang[0] * M_PI / 180) * cos(m.ang[2] * M_PI / 180) + height * cos(m.ang[0] * M_PI / 180) * sin(m.ang[1] * M_PI / 180) * sin(m.ang[2] * M_PI / 180) + m.pos[1];
		cz = height * cos(m.ang[0] * M_PI / 180) * cos(m.ang[1] * M_PI / 180) + m.pos[2];
		//std::cout << m.name << " : " << x << ", " << y << ", " << z << std::endl;
		glPushMatrix();
		{
			GLfloat col[4] = { 0.0, 0.0, 1.0, 1.0 };
			glTranslated(cx, cy, cz);
			glScaled(10.0, 10.0, 10.0);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
			glutSolidCube(1.0);
			//glutSolidSphere(1.0, 3, 3);
		}
		glPopMatrix();

		/*glPushMatrix();
		{
			GLfloat col[4] = { 1.0, 1.0, 1.0, 1.0 };
			glTranslated(tx, ty, tz);
			glScaled(10.0, 10.0, 10.0);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
			glutSolidCube(1.0);
			//glutSolidSphere(1.0, 3, 3);
		}
		glPopMatrix();*/

		tip_x.push_back(cx);
		tip_y.push_back(cy);
		tip_z.push_back(cz);
	}

	double sumTips_x = 0.0f;
	for (auto t : tip_x) {
		sumTips_x += t;
	}
	double sumTips_y = 0.0f;
	for (auto t : tip_y) {
		sumTips_y += t;
	}
	double sumTips_z = 0.0f;
	for (auto t : tip_z) {
		sumTips_z += t;
	}

	//if (tipAngle_x.size() >= 1)
	//std::cout << sumTipsAngle_x / tipAngle_x.size() << std::endl;

	//std::cout << marc.ang[1] << std::endl; 
	//std::cout << marc.ang[2] << std::endl;
	/*
	Position	average;

	for (auto t : tips) {

		glPushMatrix();
		{
			GLfloat col[4] = { 1.0, 0.0, 0.0, 1.0 };
			glTranslated(t.x, t.y, t.z);
			glScaled(10.0, 10.0, 10.0);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
			glutSolidCube(1.0);
			//glutSolidSphere(1.0, 3, 3);
		}
		glPopMatrix();

		average.x += t.x;
		average.y += t.y;
		average.z += t.z;

	}

	// ��[�ʒu�𕽋ω�
	if (tips.size() > 0) {
		average.x /= tips.size();
		average.y /= tips.size();
		average.z /= tips.size();
	}
	//�L�����u���[�V������̃}�[�J�ʒu�i�[
	cv::Mat mark_m;

	mark_m = cv::Mat::zeros(4, 1, CV_64F);
	mark_m.at<double>(0, 0) = average.x;
	mark_m.at<double>(0, 1) = average.y;
	mark_m.at<double>(0, 2) = average.z;
	mark_m.at<double>(0, 3) = 1;

	// �L�����u���[�V����
	if (m_c.rows != 0)
		mark_m *= m_c;
	*/

	double ave_c[3];
	ave_c[0] = sumTips_x / tip_x.size();
	ave_c[1] = sumTips_y / tip_y.size();
	ave_c[2] = sumTips_z / tip_z.size();
	// ��[�ʒu�̐���
	double tx = -length * cos(sumTipsAngle_y / tipAngle_x.size() * M_PI / 180) * cos(sumTipsAngle_z / tipAngle_z.size() * M_PI / 180) + ave_c[0];
	double ty = -length * cos(sumTipsAngle_y / tipAngle_x.size() * M_PI / 180) * sin(sumTipsAngle_z / tipAngle_z.size() * M_PI / 180) + ave_c[1];
	double tz = length * sin(sumTipsAngle_y / tipAngle_x.size() * M_PI / 180) + ave_c[2];
	shmem_knife->x = tx;
	shmem_knife->y = ty;
	shmem_knife->z = tz;
	//std::cout << sumTips_x / tip_x.size() << ", " << sumTips_x << ", " << tip_x.size() << std::endl;
	//std::cout << tx << ", " << ty << ", " << tz << std::endl;

	shmem_knife->xt = sumTipsAngle_x / tipAngle_x.size();
	shmem_knife->yt = sumTipsAngle_y / tipAngle_y.size();
	shmem_knife->zt = sumTipsAngle_z / tipAngle_z.size();

	glPushMatrix();
	{
		GLfloat col[4] = { 1.0, 0.0, 0.0, 1.0 };
		glTranslated(tx, ty, tz);
		glScaled(10.0, 10.0, 10.0);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
		glutSolidCube(1.0);
		//glutSolidSphere(1.0, 3, 3);
	}
	glPopMatrix();

	//std::cout << shmem_knife->x << ", " << shmem_knife->y << ", " << shmem_knife->z << std::endl;
	//std::cout << shmem_knife->xt << ", " << shmem_knife->yt << ", " << shmem_knife->zt << std::endl << std::endl;


	glutSwapBuffers();

}


/**
* �L�����u���[�V�����s��̓ǂݍ���
*/
cv::Mat loadCalibrationMatrix(std::string path) {

	cv::FileStorage fs(path, cv::FileStorage::READ);

	if (!fs.isOpened()) {
		std::cerr << "�L�����u���[�V�����s��u" << path << "�v��ǂݍ��݂܂���ł����B" << std::endl;
		return cv::Mat();
	}

	// �L�����u���[�V�����}�g���b�N�X
	cv::Mat matrix;
	fs["calibrationMatrix"] >> matrix;

	fs.release();

	return matrix;

}


int main(int argc, char* argv[]) {

	try {

		// �L�����u���[�V�����s��̓ǂݍ���
		m_c = loadCalibrationMatrix("m_ctx.xml");
		if (m_c.rows == 0) {
			// throw std::runtime_error(0);
		}

		// �}�b�s���O�I�u�W�F�N�g�쐬
		ShMem	shmem(Shm_WRITE, sizeof(SHMEM_KNIFE), TEXT("Camera"));
		shmem_knife = (SHMEM_KNIFE*)shmem.GetpAddr();

		glutInitWindowSize(WINDOW_WIDTH_SIZE, WINDOW_HEIGHT_SIZE);
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
		glutCreateWindow(WINDOW_TITLE.c_str());

		glutDisplayFunc(display);
		glutKeyboardFunc(keyboard);
		glutReshapeFunc(resize);
		glutIdleFunc(idle);

		if (mTracker.initMT() == -1) {
			throw std::runtime_error(0);
		}

		initGL();
		glutMainLoop();

	}
	catch (std::runtime_error e) {
		std::cerr << e.what() << std::endl;
	}



	std::cin.get();

	return 0;

}