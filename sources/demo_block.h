#ifndef __DEMO_BLOCK_H__
#define __DEMO_BLOCK_H__

// ""�C���N���[�h
// LightBlueEngine
#include "json_editor.h"

// �Q�[���\�[�X�t�@�C��
#include "object_block.h"

// class >> [DemoBlock]
class DemoBlock : public ObjectBlock
{
public:
	DemoBlock(bool obstacle, ObjectBoard* parent_board) : ObjectBlock(obstacle, parent_board) {};
	~DemoBlock() {};
};

#endif // __DEMO_BLOCK_H__