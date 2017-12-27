#ifndef SLIMNODE_H
#define SLIMNODE_H

#include <utility>
#include <vector>
#include <queue>
#include <algorithm>
#include <stdlib.h>

using namespace std;

typedef double stDist;

template <class T>
class SlimNode
{
public:
    enum Tipo {INDEX, LEAF};
    struct BaseElement
    {
        T m_Dato;
        stDist m_dRep;
        vector<stDist> m_dFoci;
        virtual ~BaseElement(){};
    };
protected:
    Tipo m_Tipo;
    int m_Capacity;
    struct DisjointSets
    {
        int *parent, *rnk;
        int n;
        DisjointSets(int n)
        {
            this->n = n;
            parent = new int[n+1];
            rnk = new int[n+1];

            for (int i = 0; i <= n; i++)
            {
                rnk[i] = 0;
                parent[i] = i;
            }
        }
        int FindParent(int u)
        {
            if (u != parent[u])
                parent[u] = FindParent(parent[u]);
            return parent[u];
        }
        void Merge(int x, int y)
        {
            x = FindParent(x), y = FindParent(y);
            if (rnk[x] > rnk[y])
                parent[y] = x;
            else
                parent[x] = y;

            if (rnk[x] == rnk[y])
                rnk[y]++;
        }
    };
public:
    SlimNode<T> *m_pPadre;
    vector< BaseElement* > m_Elements;
    virtual stDist GetCoverRadio() = 0;
    virtual bool SplitNode(T &d, stDist(*dist)(T,T), SlimNode<T> * Subtree = 0) = 0;
    virtual void DeleteInPadre(stDist(*dist)(T,T)) = 0;
    virtual void UpdateInfoInPadre(T d) = 0;
    virtual void DeleteElement(int Pos, stDist(*dist)(T,T)) = 0;
    virtual vector< pair< SlimNode<T> * , BaseElement * > > GetHermanos() = 0;
    virtual ~SlimNode(){};
    Tipo GetTipo()
    {
        return m_Tipo;
    }
    ///Hallar el numero de elementos en el nodo
    int GetNElements()
    {
        return m_Elements.size();
    }
    ///Verificar si puede entrar un elemento mas
    bool CheckIfFits()
    {
        return m_Elements.size() < m_Capacity;
    }
    ///Hallar la posicion del representante en el nodo
    int GetRepPos()
    {
        for (int i = 0; i < m_Elements.size(); i++)
            if (m_Elements[i]->m_dRep == 0.0)
                return i;
        return -1;
    }
    ///Prueba todas las posibilidades de representante en un nodo y retorna la posicion del elemento cuya maxima distancia hacia cualquier nodo es la menor.
    int FindBestRepPos(stDist(*dist)(T, T))
    {
        int Rep_index = 0;
        stDist Rep_MinDist = -1;
        for (int i = 0; i < m_Elements.size(); i++)
        {
            stDist MaxDistance = -1;
            for (int j = 0; j < m_Elements.size(); j++)
            {
                stDist D = dist(m_Elements[i]->m_Dato, m_Elements[j]->m_Dato);
                if (D > MaxDistance)
                    MaxDistance = D;
            }
            if (MaxDistance < Rep_MinDist || Rep_MinDist == -1)
            {
                Rep_MinDist = MaxDistance;
                Rep_index = i;
            }
        }
        return Rep_index;
    }
    ///Vuelve al elemento en la posicion Pos en el nuevo representante. Todas las distancias son recalculadas.
    void SetRep(int Pos, stDist(*dist)(T, T))
    {
        m_Elements[Pos]->m_dRep = 0.0;
        for (int i = 0; i < m_Elements.size(); i++)
            if (i != Pos)
                m_Elements[i]->m_dRep = dist(m_Elements[i]->m_Dato, m_Elements[Pos]->m_Dato);
    }
    ///Hallar el elemento con menor distancia hacia el representante (puede incluir el actual representante) y volverlo el nuevo representante.
    ///Retorna la posicion del nuevo representante
    int DefinirRep(stDist(*dist)(T, T))
    {
        int MinDist_index = -1;
        stDist MinDist = -1;
        if (!m_Elements.empty())
        {
            for (int i = 0; i < m_Elements.size(); i++)
            {
                if (MinDist == -1 || m_Elements[i]->m_dRep < MinDist)
                {
                    MinDist = m_Elements[i]->m_dRep;
                    MinDist_index = i;
                }
            }
            SetRep(MinDist_index, dist);
        }
        return MinDist_index;
    }
    ///Encuentra a un elemento dentro del nodo y retorna su posicion
    ///Si no encuentra al elemento, retorna -1
    int FindPos(T d)
    {
        if (!m_Elements.empty())
        {
            for (int i = 0; i < m_Elements.size(); i++)
                if (m_Elements[i]->m_Dato == d)
                    return i;
        }
        return -1;
    }
    ///Encuentra el elemento mas lejano del representante en un nodo y retorna su posicion
    ///Si el vector esta vacio, retorna -1
    int GetFurthestElement()
    {
        stDist D = 0.0;
        int Pos = -1;
        for (int i = 0; i < m_Elements.size(); i++)
        {
            if (m_Elements[i]->m_dRep >= D)
            {
                D = m_Elements[i]->m_dRep;
                Pos = i;
            }
        }
        return Pos;
    }
};

#endif // SLIMNODE_H
