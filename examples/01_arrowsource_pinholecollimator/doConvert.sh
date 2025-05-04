mkdir -p outputs

ConvertROOTtoLM_DoProcess -list listGAMOS.txt -conf DetParameters.txt -newEResFit -doClassification -writeAddClassificationFlag > convert.out

