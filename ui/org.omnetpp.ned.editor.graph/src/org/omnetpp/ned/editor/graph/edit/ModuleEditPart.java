package org.omnetpp.ned.editor.graph.edit;

import java.util.List;

import org.eclipse.draw2d.ConnectionAnchor;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.gef.ConnectionEditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.NodeEditPart;
import org.eclipse.gef.Request;
import org.eclipse.gef.requests.DropRequest;
import org.omnetpp.ned.editor.graph.edit.policies.NedComponentEditPolicy;
import org.omnetpp.ned.editor.graph.edit.policies.NedNodeEditPolicy;
import org.omnetpp.ned.editor.graph.figures.ModuleFigure;
import org.omnetpp.ned2.model.ConnectionNodeEx;
import org.omnetpp.ned2.model.INamedGraphNode;
import org.omnetpp.ned2.model.NEDElement;

/**
 * Base abstract controller for NedModel and NedFigures. Provides support for 
 * connection handling and common display attributes.
 */
abstract public class ModuleEditPart extends ContainerEditPart implements NodeEditPart {

	/* (non-Javadoc)
	 * @see org.eclipse.gef.editparts.AbstractGraphicalEditPart#createFigure()
	 */
	@Override
	protected IFigure createFigure() {
		// TODO Auto-generated method stub
		return null;
	}


	@Override
    protected void createEditPolicies() {
        super.createEditPolicies();
        installEditPolicy(EditPolicy.COMPONENT_ROLE, new NedComponentEditPolicy());
        installEditPolicy(EditPolicy.GRAPHICAL_NODE_ROLE, new NedNodeEditPolicy());
    }


    /**
     * Returns a list of connections for which this is the srcModule.
     * 
     * @return List of connections.
     */
    @Override
    protected List getModelSourceConnections() {
        return ((INamedGraphNode)getNEDModel()).getSrcConnections();
    }

    /**
     * Returns a list of connections for which this is the destModule.
     * 
     * @return List of connections.
     */
    @Override
    protected List getModelTargetConnections() {
        return ((INamedGraphNode)getNEDModel()).getDestConnections();
    }

    /**
     * Returns the Figure of this, as a NedFigure.
     * 
     * @return Figure as a NedFigure.
     */
    protected ModuleFigure getNedFigure() {
        return (ModuleFigure) getFigure();
    }

    /**
     * Returns the connection anchor for the given ConnectionEditPart's srcModule.
     * 
     * @return ConnectionAnchor.
     */
    public ConnectionAnchor getSourceConnectionAnchor(ConnectionEditPart connEditPart) {
    	ConnectionNodeEx conn = (ConnectionNodeEx) connEditPart.getModel();
        return getNedFigure().getConnectionAnchor(conn.getSrcGateWithIndex());
    }

    /**
     * Returns the connection anchor of a srcModule connection which is at the
     * given point.
     * 
     * @return ConnectionAnchor.
     */
    public ConnectionAnchor getSourceConnectionAnchor(Request request) {
        Point pt = new Point(((DropRequest) request).getLocation());
        return getNedFigure().getSourceConnectionAnchorAt(pt);
    }

    /**
     * Returns the connection anchor for the given ConnectionEditPart's destModule.
     * 
     * @return ConnectionAnchor.
     */
    public ConnectionAnchor getTargetConnectionAnchor(ConnectionEditPart connEditPart) {
        ConnectionNodeEx conn = (ConnectionNodeEx) connEditPart.getModel();
        return getNedFigure().getConnectionAnchor(conn.getDestGateWithIndex());
    }

    /**
     * Returns the connection anchor of a terget connection which is at the
     * given point.
     * 
     * @return ConnectionAnchor.
     */
    public ConnectionAnchor getTargetConnectionAnchor(Request request) {
        Point pt = new Point(((DropRequest) request).getLocation());
        return getNedFigure().getTargetConnectionAnchorAt(pt);
    }

    /**
     * Returns the gate associated with a the given connection anchor.
     * 
     * @return The name of the ConnectionAnchor as a String.
     */
    final public String getGate(ConnectionAnchor c) {
        return getNedFigure().getGate(c);
    }

    /* (non-Javadoc)
	 * @see org.omnetpp.ned.editor.graph.edit.ContainerEditPart#attributeChanged(org.omnetpp.ned2.model.NEDElement, java.lang.String)
	 */
	@Override
	public void attributeChanged(NEDElement node, String attr) {
		super.attributeChanged(node, attr);
		
		// SubmoduleNodeEx and CompoundModuleNodeEx fire ATT_SRC(DEST)_CONNECTION 
		// attribute change if a connection gets added/removed
		if (INamedGraphNode.ATT_SRC_CONNECTION.equals(attr))
			refreshSourceConnections();
		else if (INamedGraphNode.ATT_DEST_CONNECTION.equals(attr))
			refreshTargetConnections();
	}

}
