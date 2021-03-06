/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::compressible::
        wallCondensationCoupledMixedFvPatchScalarField

Description
    Mixed boundary condition for temperature and radiation heat transfer
    to be used for in multiregion cases. Optional thin thermal layer
    resistances can be specified through thicknessLayers and kappaLayers
    entries.

    The thermal conductivity \c kappa can either be retrieved from various
    possible sources, as detailed in the class temperatureCoupledBase.

Usage
    \table
        Property     | Description             | Required    | Default value
        Tnbr         | name of the field    | no | T
        qrNbr      | name of the radiative flux in the nbr region | no | none
        qr         | name of the radiative flux in this region | no | none
        thicknessLayers | list of thicknesses per layer [m] | no |
        kappaLayers  | list of thermal conductivites per layer [W/m/K] | no |
        kappaMethod  | inherited from temperatureCoupledBase | inherited |
        kappa        | inherited from temperatureCoupledBase | inherited |
        TODO: need to add description
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            compressible::wallCondensationCoupledMixed;
        Tnbr            T;
        qrNbr           qr; // or none. Name of qr field on neighbour region
        qr              qr; // or none. Name of qr field on local region
        thicknessLayers (0.1 0.2 0.3 0.4);
        kappaLayers     (1 2 3 4);
        kappaMethod     lookup;
        kappa           kappa;
        value           uniform 300;
        TODO: need to add description
    }
    \endverbatim

    Needs to be on underlying mapped(Wall)FvPatch.

See also
    Foam::temperatureCoupledBase

SourceFiles
    wallCondensationCoupledMixedFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef wallCondensationCoupledMixedFvPatchScalarField_H
#define wallCondensationCoupledMixedFvPatchScalarField_H

#include "mixedFvPatchFields.H"
#include "temperatureCoupledBase.H"
#include "scalarList.H"
#include "volFields.H"
#include "liquidProperties.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace compressible
{

/*---------------------------------------------------------------------------*\
    Class wallCondensationCoupledMixedFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class wallCondensationCoupledMixedFvPatchScalarField
:
    public mixedFvPatchScalarField,
    public temperatureCoupledBase
{
private:
    // Private data

        //- Name of field on the neighbour region
        const word TnbrName_;

         //- Name of the radiative heat flux in the neighbor region
        const word qrNbrName_;

        //- Name of the radiative heat flux in local region
        const word qrName_;

        //- Thickness of layers
        scalarList thicknessLayers_;

        //- Conductivity of layers
        scalarList kappaLayers_;

        //- Total contact resistance
        scalar contactRes_;

        //- Name of the species on which the mass transferred (default H2O)
        const word specieName_;

        //- Liquid properties
        autoPtr<liquidProperties> liquid_;

        //- Liquid dictionary
        dictionary liquidDict_;

        //- Mass accumulated on faces
        scalarField mass_;

        //- Mass accumulated on faces up to last time step
        scalarField massOld_;

        //- Cache myDelta
        scalarField myKDelta_;

        //- Phase change energy
        scalarField dmHfg_;

        //- Thermal inertia
        scalarField mpCpTp_;
        
        //- Average molecular weight for the carrier mixture in the gas phase
        scalar Mcomp_;

        //- Fluid side
        bool fluid_;

        //- Thickness field for inert mode
        scalarField thickness_;

        //- Last time step for which mass of fluid film was calculated
        scalar lastTimeStep_;

    // Private members
        //- Lookup (or create) a field for output
        volScalarField& outputScalarField
        (
            const word& fieldName,
            const dimensionSet& dimSet,
            const fvMesh& mesh
        );


public:

    //- Runtime type information
    TypeName("compressible::wallCondensationCoupledMixed");


    // Constructors

        //- Construct from patch and internal field
        wallCondensationCoupledMixedFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        wallCondensationCoupledMixedFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  turbulentTemperatureCoupledBaffleMixedFvPatchScalarField onto a
        //  new patch
        wallCondensationCoupledMixedFvPatchScalarField
        (
            const
            wallCondensationCoupledMixedFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new wallCondensationCoupledMixedFvPatchScalarField
                (
                    *this
                )
            );
        }

        //- Construct as copy setting internal field reference
        wallCondensationCoupledMixedFvPatchScalarField
        (
            const wallCondensationCoupledMixedFvPatchScalarField&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchScalarField> clone
        (
            const DimensionedField<scalar, volMesh>& iF
        ) const
        {
            return tmp<fvPatchScalarField>
            (
                new wallCondensationCoupledMixedFvPatchScalarField
                (
                    *this,
                    iF
                )
            );
        }


    // Member functions

    // Mapping functions

        //- Map (and resize as needed) from self given a mapping object
        virtual void autoMap
        (
            const fvPatchFieldMapper&
        );

        //- Reverse map the given fvPatchField onto this fvPatchField
        virtual void rmap
        (
            const fvPatchScalarField&,
            const labelList&
        );


        //- Return myKDelta
        const scalarField myKDelta() const
        {
            return myKDelta_;
        }

        //- Return mpCpTp
        const scalarField mpCpTp() const
        {
            return mpCpTp_;
        }

        //- Return dmHfg
        const scalarField dmHfg() const
        {
            return dmHfg_;
        }

        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace compressible
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
