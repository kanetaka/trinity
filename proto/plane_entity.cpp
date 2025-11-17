#include "plane_entity.h"
#include "visualizer.h"
#include "renderer.h"
#include "mesh_component.h"

PlaneEntity::PlaneEntity(Visualizer* vis): Entity(vis)
{
	SetScale(10.0f);
	MeshComponent* mc = new MeshComponent(this);
	mc->SetMesh(GetVisualizer()->GetRenderer()->GetMesh("assets/plane.gpmesh"));
}
