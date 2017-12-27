#ifndef SLIMNODELEAF_H
#define SLIMNODELEAF_H

#include "slimnodeidx.h"

template <class T>
class SlimNodeLeaf: public SlimNode<T>
{
public:
    struct LeafElement: public SlimNode<T>::BaseElement
    {
        LeafElement(T d, stDist distanceRep)
        {
            SlimNode<T>::BaseElement::m_Dato = d;
            SlimNode<T>::BaseElement::m_dRep = distanceRep;
        }
    };
    SlimNodeLeaf(int C, SlimNodeIdx<T> * padre = 0)
    {
        SlimNode<T>::m_Tipo = SlimNode<T>::LEAF;
        SlimNode<T>::m_Capacity = C;
        SlimNode<T>::m_pPadre = padre;
    }
    ///Retorna la mayor distancia al representante en el nodo
    stDist GetCoverRadio()
    {
        stDist D = 0.0;
        for (auto it = SlimNode<T>::m_Elements.begin(); it != SlimNode<T>::m_Elements.end(); it++)
            if ((*it)->m_dRep > D)
                D = (*it)->m_dRep;
        return D;
    }
    ///Halla la posicion del representante de este nodo en su padre, y lo elimina.
    void DeleteInPadre(stDist (*dist)(T,T))
    {
        T Rep_data = SlimNode<T>::m_Elements[SlimNode<T>::GetRepPos()]->m_Dato;
        int PosInPadre = SlimNode<T>::m_pPadre->FindPos(Rep_data);
        static_cast< SlimNodeIdx<T> * >(SlimNode<T>::m_pPadre)->DeleteElement(PosInPadre, dist);
    }
    ///Halla la posicion del representante de este nodo en su padre, y actualiza su informacion (radio y numero de elementos).
    void UpdateInfoInPadre(T d)
    {
        int PosInPadre = SlimNode<T>::m_pPadre->FindPos(d);
        static_cast< typename SlimNodeIdx<T>::IdxElement * >(SlimNode<T>::m_pPadre->m_Elements[PosInPadre])->UpdateSTInfo();
        if (SlimNode<T>::m_pPadre->m_pPadre) SlimNode<T>::m_pPadre->UpdateInfoInPadre(SlimNode<T>::m_pPadre->m_Elements[SlimNode<T>::m_pPadre->GetRepPos()]->m_Dato);
    }
    vector< pair< SlimNode<T> * , typename SlimNode<T>::BaseElement * > > GetHermanos()
    {
        vector< pair< SlimNode<T> * , typename SlimNode<T>::BaseElement * > > Hermanos;
        if (SlimNode<T>::m_pPadre)
        {
            int PosInPadre = SlimNode<T>::m_pPadre->FindPos(SlimNode<T>::m_Elements[SlimNode<T>::GetRepPos()]->m_Dato);
            for (int i = 0; i < SlimNode<T>::m_pPadre->m_Elements.size(); i++)
                if (i != PosInPadre)
                    Hermanos.push_back(pair< SlimNode<T> * , typename SlimNode<T>::BaseElement * >(static_cast< typename SlimNodeIdx<T>::IdxElement * >(SlimNode<T>::m_pPadre->m_Elements[i])->m_pSubtree, SlimNode<T>::m_pPadre->m_Elements[i]));
        }
        return Hermanos;
    }
    ///Crea un grafo con todos los elementos del nodo + elemento que se quiere agregar.
    ///Crear todas las aristas posibles y las ordena por peso.
    ///Halla el MST.
    ///Elimina la arista mas grande.
    ///Crea los dos nuevos nodos con los dos grupos de elementos, halla sus representantes y retorna los nodos.
    pair< pair< SlimNodeLeaf<T>*, int >, pair< SlimNodeLeaf<T>*, int > > KruskalMST(LeafElement *e, stDist(*dist)(T,T), SlimNodeIdx<T> * padre = 0)
    {
        ///Armar el grafo con todas las aristas posibles
        vector< stEdge > Edges;
        for (int i = 0; i < SlimNode<T>::m_Elements.size()+1; i++)
        {
            T a, b;
            if (i == SlimNode<T>::m_Elements.size()) a = e->m_Dato;
            else a = SlimNode<T>::m_Elements[i]->m_Dato;
            for (int j = i+1; j < SlimNode<T>::m_Elements.size()+1; j++)
            {
                if (j == SlimNode<T>::m_Elements.size()) b = e->m_Dato;
                else b = SlimNode<T>::m_Elements[j]->m_Dato;
                Edges.push_back(pair< stDist, pair<int, int> >(dist(a,b), pair< int, int >(i,j)));
            }
        }
        ///Una vez se tengan las aristas, ordenarlas y ejecutar Kruskal
        sort(Edges.begin(), Edges.end(), [&](stEdge &A, stEdge &B) -> bool {return A.first < B.first;});
        typename SlimNode<T>::DisjointSets DS(SlimNode<T>::m_Elements.size()+1);
        vector< pair<int, int> > MSTEdges;
        for (auto it = Edges.begin(); it != Edges.end(); it++)
        {
            int A = it->second.first;
            int B = it->second.second;
            int Parent_A = DS.FindParent(A);
            int Parent_B = DS.FindParent(B);
            if (Parent_A != Parent_B)
            {
                MSTEdges.push_back(pair<int,int>(A, B));
                DS.Merge(Parent_A, Parent_B);
            }
        }
        ///Eliminar la arista mas larga (MSTEdges esta ordenado por distancia ascendente). Crear los nuevos nodos.
        int N1Start = MSTEdges.back().first;
        int N2Start = MSTEdges.back().second;
        MSTEdges.pop_back();
        SlimNodeLeaf<T> * N1 = new SlimNodeLeaf<T>(SlimNode<T>::m_Capacity, padre);
        SlimNodeLeaf<T> * N2 = new SlimNodeLeaf<T>(SlimNode<T>::m_Capacity, padre);
        ///Agregar los elementos al Nodo1
        queue<int> VerticesN1;
        VerticesN1.push(N1Start);
        while (!VerticesN1.empty())
        {
            int V = VerticesN1.front();
            if (V == SlimNode<T>::m_Elements.size()) N1->m_Elements.push_back(e);
            else N1->m_Elements.push_back(SlimNode<T>::m_Elements[V]);
            int edge = 0;
            while (edge < MSTEdges.size())
            {
                if (MSTEdges[edge].first == V)
                {
                    VerticesN1.push(MSTEdges[edge].second);
                    MSTEdges.erase(MSTEdges.begin() + edge);
                }
                else if (MSTEdges[edge].second == V)
                {
                    VerticesN1.push(MSTEdges[edge].first);
                    MSTEdges.erase(MSTEdges.begin() + edge);
                }
                else edge++;
            }
            VerticesN1.pop();
        }
        ///Agregar los elementos al Nodo2
        queue<int> VerticesN2;
        VerticesN2.push(N2Start);
        while (!VerticesN2.empty())
        {
            int V = VerticesN2.front();
            if (V == SlimNode<T>::m_Elements.size()) N2->m_Elements.push_back(e);
            else N2->m_Elements.push_back(SlimNode<T>::m_Elements[V]);
            int edge = 0;
            while (edge < MSTEdges.size())
            {
                if (MSTEdges[edge].first == V)
                {
                    VerticesN2.push(MSTEdges[edge].second);
                    MSTEdges.erase(MSTEdges.begin() + edge);
                }
                else if (MSTEdges[edge].second == V)
                {
                    VerticesN2.push(MSTEdges[edge].first);
                    MSTEdges.erase(MSTEdges.begin() + edge);
                }
                else edge++;
            }
            VerticesN2.pop();
        }
        ///Hallar el mejor representante para Nodo1 y Nodo2, y volverlos representantes.
        int N1RepPos = N1->FindBestRepPos(dist);
        int N2RepPos = N2->FindBestRepPos(dist);
        N1->SetRep(N1RepPos, dist);
        N2->SetRep(N2RepPos, dist);

        return pair< pair< SlimNodeLeaf<T>*, int >, pair< SlimNodeLeaf<T>*, int > >(pair< SlimNodeLeaf<T>*, int >(N1, N1RepPos), pair< SlimNodeLeaf<T>*, int >(N2, N2RepPos));
    }
    ///Hacer split al nodo: Separar en dos grupos, eliminar el representante en el padre (si tiene) y agregar los nuevos representantes
    bool SplitNode(T &d, stDist(*dist)(T,T), SlimNode<T> * Subtree = 0)
    {
        LeafElement * element = new LeafElement(d, 0);
        ///Si no tiene padre, se debe crear un nuevo nodo para agregar ahi a los nuevos representantes.
        if (!SlimNode<T>::m_pPadre)
        {
            SlimNodeIdx<T> * R = new SlimNodeIdx<T>(SlimNode<T>::m_Capacity);
            SlimNode<T>::m_pPadre = R;
            pair< pair< SlimNodeLeaf<T>*, int >, pair< SlimNodeLeaf<T>*, int > > NewNodes = KruskalMST(element, dist, R);
            R->AddElement(NewNodes.first.first->m_Elements[NewNodes.first.second]->m_Dato, dist, NewNodes.first.first);
            R->AddElement(NewNodes.second.first->m_Elements[NewNodes.second.second]->m_Dato, dist, NewNodes.second.first);
            return true;
        }
        ///Si tiene padre, pero este tiene solo un elemento, debemos anadir uno de los nuevos representantes primero para evitar que el nodo se quede vacio.
        ///Un nodo vacio no puede hallar su nuevo representante, por lo que no podemos reemplazar hacia arriba.
        if (SlimNode<T>::m_pPadre->m_Elements.size() == 1)
        {
            int PosInPadre = SlimNode<T>::m_pPadre->FindPos(SlimNode<T>::m_Elements[SlimNode<T>::GetRepPos()]->m_Dato);
            pair< pair< SlimNodeLeaf<T>*, int >, pair< SlimNodeLeaf<T>*, int > > NewNodes = KruskalMST(element, dist, static_cast< SlimNodeIdx<T> * >(SlimNode<T>::m_pPadre));
            static_cast< SlimNodeIdx<T>* >(SlimNode<T>::m_pPadre)->AddElement(NewNodes.first.first->m_Elements[NewNodes.first.second]->m_Dato, dist, NewNodes.first.first);
            static_cast< SlimNodeIdx<T>* >(SlimNode<T>::m_pPadre)->DeleteElement(PosInPadre, dist);
            return static_cast< SlimNodeIdx<T>* >(SlimNode<T>::m_pPadre)->AddElement(NewNodes.second.first->m_Elements[NewNodes.second.second]->m_Dato, dist, NewNodes.second.first);
        }
        ///Si tiene padre y este tiene mas de un elemento, se elimina el antiguo representante y se agregan los dos nuevos.
        else
        {
            DeleteInPadre(dist);
            pair< pair< SlimNodeLeaf<T>*, int >, pair< SlimNodeLeaf<T>*, int > > NewNodes = KruskalMST(element, dist, static_cast< SlimNodeIdx<T> * >(SlimNode<T>::m_pPadre));
            static_cast< SlimNodeIdx<T>* >(SlimNode<T>::m_pPadre)->AddElement(NewNodes.first.first->m_Elements[NewNodes.first.second]->m_Dato, dist, NewNodes.first.first);
            return static_cast< SlimNodeIdx<T>* >(SlimNode<T>::m_pPadre)->AddElement(NewNodes.second.first->m_Elements[NewNodes.second.second]->m_Dato, dist, NewNodes.second.first);
        }
    }
    ///Verificar si puede caber un elemento mas.
    ///Si puede, agregar el elemento y si es que este nodo tiene padre, actualizar la informacion del representante de este nodo en su padre.
    ///Si no hay espacio, hacer split.
    ///Retorna true si se hizo split a la raiz (el arbol aumenta de nivel)
    bool AddElement(T &d, stDist(*dist)(T, T))
    {
        if (SlimNode<T>::CheckIfFits())
        {
            stDist distanceRep = 0.0;
            if (!SlimNode<T>::m_Elements.empty()) distanceRep = dist(d, SlimNode<T>::m_Elements[SlimNode<T>::GetRepPos()]->m_Dato);
            LeafElement * element = new LeafElement(d, distanceRep);
            SlimNode<T>::m_Elements.push_back(element);
            if (SlimNode<T>::m_pPadre) UpdateInfoInPadre(SlimNode<T>::m_Elements[SlimNode<T>::GetRepPos()]->m_Dato);
            return false;
        }
        return SplitNode(d, dist);
    }
    void DeleteElement(int Pos, stDist(*dist)(T, T))
    {
        T temp_data = SlimNode<T>::m_Elements[Pos]->m_Dato;
        bool Rep = SlimNode<T>::GetRepPos() == Pos;
        SlimNode<T>::m_Elements.erase(SlimNode<T>::m_Elements.begin() + Pos);
        if (SlimNode<T>::m_pPadre && Rep) UpdateInfoInPadre(temp_data);
        if (SlimNode<T>::m_Elements.empty())
        {
            if (SlimNode<T>::m_pPadre)
            {
                Pos = SlimNode<T>::m_pPadre->FindPos(temp_data);
                SlimNode<T>::m_pPadre->DeleteElement(Pos, dist);
            }
            delete this;
            return;
        }
        if (Rep)
        {
            int NuevoRep = SlimNode<T>::DefinirRep(dist);
            if (SlimNode<T>::m_pPadre)
            {
                Pos = SlimNode<T>::m_pPadre->FindPos(temp_data);
                static_cast< SlimNodeIdx<T> * >(SlimNode<T>::m_pPadre)->ReplaceElement(Pos, SlimNode<T>::m_Elements[NuevoRep]->m_Dato, dist);
            }
        }
        else if (SlimNode<T>::m_pPadre) UpdateInfoInPadre(SlimNode<T>::m_Elements[SlimNode<T>::GetRepPos()]->m_Dato);
    }
};

#endif // SLIMNODELEAF_H
