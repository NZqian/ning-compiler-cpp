#include "AST.hpp"
#include "../IR/three_address.hpp"
#include "../frontend/symtable.hpp"

Visitor::Visitor()
{
    indent = "";
    symtable = std::make_shared<SymTable>();
    threeAddressCode = std::make_shared<ThreeAddressCode>();
    nameChanger = std::make_shared<NameChanger>();
    inLoop = false;
}

void FillArray(std::shared_ptr<ArrayAST> &arr, std::vector<std::shared_ptr<BaseAST>> dimensions, int depth)
{
    if (arr->items.size())
    {
        std::shared_ptr<ArrayAST> arrBack = std::dynamic_pointer_cast<ArrayAST>(arr->items.back());
        FillArray(arrBack, dimensions, depth + 1);
    }
    for (int j = arr->items.size(); j < dimensions[depth]->GetVal(); j++)
    {
        if (depth == dimensions.size() - 1)
        {
            arr->items.push_back(std::make_shared<LiteralAST>(0));
        }
        else
        {
            arr->items.push_back(std::make_shared<ArrayAST>());
            std::shared_ptr<ArrayAST> arr1 = std::dynamic_pointer_cast<ArrayAST>(arr->items[j]);
            FillArray(arr1, dimensions, depth + 1);
        }
    }
}

void GetArrayLast(std::shared_ptr<BaseAST> &arr, int depth)
{
}

//深度优先遍历原数组结构，只调节本层
void TraverseArray(std::shared_ptr<BaseAST> arr, int depth, std::vector<std::shared_ptr<BaseAST>> dimensions, std::shared_ptr<BaseAST> &newVal)
{
    std::shared_ptr<ArrayAST> realArr = std::dynamic_pointer_cast<ArrayAST>(arr);
    std::shared_ptr<ArrayAST> realNewArr = std::dynamic_pointer_cast<ArrayAST>(newVal);
    //std::cout << arr->TypeName();
    if (arr->TypeName() == typeid(ArrayAST).name())
    {
        for (auto item : realArr->items)
        {

            //std::cout << realNewArr->items.size() << std::endl;
            //std::cout << std::dynamic_pointer_cast<ArrayAST>(realNewArr->items.back())->items.size() << std::endl;
            if (!realNewArr->items.size() ||
                (dimensions.size() > 1 && realNewArr->items.size() != dimensions[depth]->GetVal()))
            {
                realNewArr->items.push_back(std::make_shared<ArrayAST>());
            }
            TraverseArray(item, depth + 1, dimensions, realNewArr->items.back());
        }
    }
    else
    {
        if (depth == dimensions.size() - 1)
        {
            newVal = arr;
        }
        else
        {
            if (!realNewArr->items.size())
            {
                
            }

        }
    }
}

void VariableAST::UpdateIndex(int depth)
{
    int allPositions, usedPositions;
    int changeFlag = false;
    for (int i = depth + 1; i < index.size(); i++)
    {
        if (index[i] > 0) 
        {
            changeFlag = true;
        }
    }
    if (depth == index.size() - 1)
    {
        changeFlag = true;
    }
    if (changeFlag)
    {
        int carry = 1;
        for (int i = depth; i >= 0; i--)
        {
            int tmp = carry;
            carry = (index[i] + carry) / dimensions[i]->GetVal();
            index[i] = (index[i] + tmp) % dimensions[i]->GetVal();
        }
        for (int i = depth + 1; i < index.size(); i++)
        {
            index[i] = 0;
        }
    }
}

void VariableAST::GetNextIndexBlock(int depth)
{
    int carry = 1;
    for (int i = depth; i >= 0; i--)
    {
        int tmp = carry;
        carry = (index[i] + carry) / dimensions[i]->GetVal();
        index[i] = (index[i] + tmp) % dimensions[i]->GetVal();
    }
}

void VariableAST::GetNextIndex()
{
    int carry = 1;
    for (int i = index.size() - 1; i >= 0; i--)
    {
        int tmp = carry;
        carry = (index[i] + carry) / dimensions[i]->GetVal();
        index[i] = (index[i] + tmp) % dimensions[i]->GetVal();
    }
}

void VariableAST::TraverseArray(std::shared_ptr<BaseAST> arr, std::shared_ptr<BaseAST> &newVal, int depth, std::vector<int> dimensions)
{
    if (arr->TypeName() == typeid(ArrayAST).name())
    {
        UpdateIndex(depth);
        if (!std::dynamic_pointer_cast<ArrayAST>(arr)->items.size())
        {
            GetNextIndexBlock(depth);
        }
        else
        {
            for (auto item : std::dynamic_pointer_cast<ArrayAST>(arr)->items)
            {
                TraverseArray(item, newVal, depth + 1, dimensions);
            }
            UpdateIndex(depth);
        }
    }
    else
    {
        std::shared_ptr<ArrayAST> tmparr = std::dynamic_pointer_cast<ArrayAST>(newVal);
        for (int i = 0; i < index.size() - 1; i++)
        {
            while(tmparr->items.size() < index[i] + 1)
            {
                tmparr->items.push_back(std::make_shared<ArrayAST>());
            }
            tmparr = std::dynamic_pointer_cast<ArrayAST>(tmparr->items[index[i]]);
        }
        while(tmparr->items.size() < index.back())
        {
            tmparr->items.push_back(std::make_shared<LiteralAST>(0));
        }
        tmparr->items.push_back(arr);
        GetNextIndex();
    }
}

void VariableAST::FillEmpty(std::shared_ptr<BaseAST> &arr, int depth)
{
    for (int i = 0; i < dimensions[depth]->GetVal(); i++)
    {
        if (i < std::dynamic_pointer_cast<ArrayAST>(arr)->items.size())
        {
            if (depth != dimensions.size() - 1)
            {
                FillEmpty(std::dynamic_pointer_cast<ArrayAST>(arr)->items[i], depth + 1);
            }
            else
            {
                continue;
            }
        }
        else
        {
            //最底层
            if (depth == dimensions.size() - 1)
            {
                std::dynamic_pointer_cast<ArrayAST>(arr)->items.push_back(std::make_shared<LiteralAST>(0));
            }
            else
            {
                std::dynamic_pointer_cast<ArrayAST>(arr)->items.push_back(std::make_shared<ArrayAST>());
                FillEmpty(std::dynamic_pointer_cast<ArrayAST>(arr)->items.back(), depth + 1);
            }
        }
    }
}

void VariableAST::ReconstructArr()
{
    std::vector<int> intDimensions;
    for (int i = 0; i < dimensions.size(); i++)
    {
        intDimensions.push_back(dimensions[i]->GetVal());
    }

    index = std::vector<int>(dimensions.size(), 0);
    //设为－１，因为在获取下一位置时会加一
    //index.back() = -1;

    std::shared_ptr<BaseAST> newArr = std::make_shared<ArrayAST>();
    //VariableAST::TraverseArray(val, newArr, 0, intDimensions);
    TraverseArray(val, newArr, -1, intDimensions);

    std::shared_ptr<ArrayAST> tmparr = std::dynamic_pointer_cast<ArrayAST>(newArr);
    val = newArr;
    FillEmpty(val, 0);
}

std::string VariableAST::GetName()
{
    //数组元素
    if (dimensions.size() && !val)
    {
        std::string newName = name;
        for (auto dimension : dimensions)
        {
            newName += "$";
            if (dimension->TypeName() == typeid(LiteralAST).name())
            {
                newName += std::to_string(((LiteralAST*)dimension.get())->val);
            }
            else
            {
                newName += ((VariableAST*)dimension.get())->GetName();
            }
        }
        return newName;
    }
    else
    {
        return name;
    }
}